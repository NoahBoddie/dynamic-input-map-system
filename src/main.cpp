

using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

//using namespace SOS;
//using namespace RGL;



#include "Hooks.h"

#include "xbyak/xbyak.h"



using namespace DIMS;


void InitializeLogging()
{

    auto path = log_directory();
    if (!path) {
        report_and_fail("Unable to lookup SKSE logs directory.");
    }
    *path /= PluginDeclaration::GetSingleton()->GetName();
    *path += L".log";

    std::shared_ptr<spdlog::logger> log;
    if (IsDebuggerPresent()) {
        log = std::make_shared<spdlog::logger>(
            "Global", std::make_shared<spdlog::sinks::msvc_sink_mt>());
    }
    else {
        log = std::make_shared<spdlog::logger>(
            "Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));
    }


#ifndef NDEBUG
    const auto level = spdlog::level::trace;
#else
    //Use right alt for just debug logging, control to allow debugger to attach.
    const auto level = GetKeyState(VK_RCONTROL) & 0x800 || GetKeyState(VK_RMENU) & 0x800 ?
        spdlog::level::debug : spdlog::level::info;
#endif


    log->set_level(level);
    log->flush_on(level);

    spdlog::set_default_logger(std::move(log));
    //spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] %v");
    spdlog::set_pattern("%s(%#): [%^%l%$] %v"s);


#ifdef NDEBUG
    if (spdlog::level::debug == level) {
        logger::debug("debug logger in release enabled.");
    }
#endif
}

using EventSource = RE::BSTEventSource<RE::InputEvent*>;
using EventSink = RE::BSTEventSink<RE::InputEvent*>;

inline RE::BSTEventSink<RE::InputEvent*>* skseSink = nullptr;


void InitializeMessaging() {
    if (!GetMessagingInterface()->RegisterListener([](MessagingInterface::Message* message) {
        switch (message->type) {
        case MessagingInterface::kInputLoaded: {
            auto controls = RE::PlayerControls::GetSingleton();
            auto& sinks = RE::BSInputDeviceManager::GetSingleton()->sinks;

            for (int i = 0; i < sinks.size(); i++)
            {
                if (sinks[i] == controls) {
                    assert(sinks.size() > i + 1);
                    skseSink = sinks[i + 1];
                    logger::info("SKSE sink found.");
                    break;
                }

            }

            break;
            // It is now safe to do multithreaded operations, or operations against other plugins.
        }

        case MessagingInterface::kPostPostLoad: // Called after all kPostLoad message handlers have run.

            break;

        case MessagingInterface::kDataLoaded:
            CONTROLESQUE(RE::ControlMap::GetSingleton());
            break;
        }
        })) {
        stl::report_and_fail("Unable to register message listener.");
    }
}


struct InputHook
{
    static void Install()
    {
        //704DE0+49
        REL::Relocation<uintptr_t> hook{ REL::RelocationID{41259, 0}, 0x49 };
    
        func = SKSE::GetTrampoline().write_call<5>(hook.address(), thunk);
        
        logger::debug("hook success.");
    }



    static void thunk(RE::PlayerControls* a_controls, RE::InputEvent* a_event)
    {
        std::unique_ptr<RE::InputEvent> out;
        //RE::BSInputEventQueue;

        if (DIMS::testController->HandleEvent({ a_event, a_controls }, out) == false)
        {
            return;
        }

        if (auto id_event = a_event->AsIDEvent())
        {
            auto device = id_event->device.get();
            auto e_type = id_event->eventType.get();
            auto code = id_event->GetIDCode();
            auto str = id_event->userEvent;

            if (auto button = a_event->AsButtonEvent())
            {
                if (button->value == 0)
                {
                    //return;
                }
            }

            if (auto thumb_event = a_event->AsThumbstickEvent())
            {
                auto x = thumb_event->xValue;
                auto y = thumb_event->yValue;
                
            }

            return func(a_controls, out ? out.get() : a_event);
        }

        return func(a_controls, out ? out.get() : a_event);
    }

    inline static REL::Relocation<decltype(thunk)> func;
};



struct ReleaseHook
{
    static void Install()
    {

        //704DE0+A3
        REL::Relocation<uintptr_t> hook{ REL::RelocationID{41259, 0}, 0xA3 };

        func = SKSE::GetTrampoline().write_call<5>(hook.address(), thunk);

        logger::debug("hook success.");
    }



    static void thunk(RE::PlayerControls* a_controls)
    {
        DIMS::testController->HandleRelease(a_controls);
        return func(a_controls);    
    }

     

    inline static REL::Relocation<decltype(thunk)> func;
};

//prologue
struct QueueReleaseHook
{
    static void Install()
    {
        //SE: 0x705960, AE: 0x000000, VR: ???
        auto hook_addr = REL::RelocationID(41273, 00000).address();
        auto ret_addr = hook_addr + 0xB;
        struct Code : Xbyak::CodeGenerator
        {
            Code(uintptr_t ret_addr)
            {
                sub(rsp, 0x28);
                cmp(byte[rcx + 0xA0], 0);

                mov(rax, ret_addr);
                jmp(rax);
               
            }
        } static code{ ret_addr };

        auto& trampoline = SKSE::GetTrampoline();

        auto placed_call = IsCallOrJump(hook_addr) > 0;

        auto place_query = trampoline.write_branch<5>(hook_addr, (uintptr_t)thunk);

        if (!placed_call)
            func = (uintptr_t)code.getCode();
        else
            func = place_query;


        logger::info("QueueReleaseHook complete...");
    }

    static void thunk(RE::PlayerControls* a_this)
    {
        //*This check might only work for other inputs, not these.
        //if (a_this->unk0A0[0] != 0)
        DIMS::testController->QueueRelease();

        func(a_this);
    }

    inline static REL::Relocation<decltype(thunk)> func;
};


struct PlayerCharacter_GraphOutputEvent
{
    static void Install()
    {
        REL::Relocation<uintptr_t> Player_GraphEvent_Vtbl{ RE::VTABLE_PlayerCharacter[2] };
        
        func = Player_GraphEvent_Vtbl.write_vfunc(0x1, thunk);
    }



    static RE::BSEventNotifyControl thunk(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_this, RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_source)
    {
        testController->stateMap.Update(RefreshCode::GraphOutputEvent, a_event->tag);
        return func(a_this, a_event, a_source);
    }

    inline static REL::Relocation<decltype(thunk)> func;
};

//Please note, this needs to be very much so last, so it knows the true result of it's actions.
struct PlayerCharacter_GraphInputEvent
{
    static void Install()
    {
        REL::Relocation<uintptr_t> Player_GraphManager_Vtbl{ RE::VTABLE_PlayerCharacter[3] };

        func = Player_GraphManager_Vtbl.write_vfunc(0x1, thunk);
    }
    
    static bool thunk(RE::IAnimationGraphManagerHolder* a_this, const RE::BSFixedString& a_name)
    {
        auto result = func(a_this, a_name);

        if (result)
            testController->stateMap.Update(RefreshCode::GraphInputEvent, a_name);

        return result;
    }

    inline static REL::Relocation<decltype(thunk)> func;
};


struct SKSEInputHook
{
    static void Install()
    {
        //SE: 0xC15E00, AE: 0x000000, VR: ???
        auto hook_addr = REL::RelocationID(67355, 00000).address() + 0x172;
        
        struct Code : Xbyak::CodeGenerator
        {
            Code(uintptr_t func, uintptr_t ret_addr)
            {
                mov(rax, func);
                call(rax);
                mov(ebp, eax);
                mov(rax, ret_addr);
                jmp(rax);
                //ret();
            }
        } static code{ (uintptr_t)thunk, hook_addr + 0x5 };
        
        auto& trampoline = SKSE::GetTrampoline();

        auto placed_call = IsCallOrJump(hook_addr) > 0;

        //We can use write_call due to having just enough space to return.
        auto place_query = trampoline.write_branch<5>(hook_addr, (uintptr_t)code.getCode());

        //if (!placed_call)
        //    func = (uintptr_t)code.getCode();
        //else
        //    func = place_query;


        logger::info("SKSEInputHook complete...");
    }

    static RE::BSEventNotifyControl thunk(EventSink* a_this, RE::InputEvent** inputs, EventSource* source)
    {
        //This shit is messy sure, but I'll find a better way to do this eventually.

        //Note, this is not only temporary, it's basically untenable.

        //I also want this to work on other things registering for keys, just nothing from skyrim.

        bool skse = false;
        
        
        std::vector<std::unique_ptr<RE::InputEvent>> queue;

        RE::InputEvent** overrides;
        RE::InputEvent* input = *inputs;

        
        if (skseSink && a_this == skseSink && InputQueue::HasQueue() == true) {
            skse = true;

            queue = InputQueue::MoveQueue();

            if (*inputs)
            {   
                overrides = inputs;


                while (input->next) input = input->next;

                input->next = queue.front().get();
            }
            else
            {
                overrides = reinterpret_cast<RE::InputEvent**>(queue.data());
            }
        }
        else
        {
            overrides = inputs;
        }
       

        

        auto result = a_this->ProcessEvent(overrides, source);
        
        if (skse && input) {
            input->next = nullptr;
        }

        return result;
    }

    inline static REL::Relocation<decltype(thunk)> func;
};







SKSEPluginLoad(const LoadInterface* skse) {
    InitializeLogging();
    
#ifdef _DEBUG

    

    if (GetKeyState(VK_RCONTROL) & 0x800) {
        constexpr auto text1 = L"Request for debugger detected. If you wish to attach one and press Ok, do so now if not please press Cancel.";
        constexpr auto text2 = L"Debugger still not detected. If you wish to continue without one please press Cancel.";
        constexpr auto caption = L"Debugger Required (DIMS)";

        int input = 0;

        do
        {
            input = MessageBox(NULL, !input ? text1 : text2, caption, MB_OKCANCEL);
        } while (!IsDebuggerPresent() && input != IDCANCEL);
    }


    if (GetKeyState(VK_RCONTROL) & 0x800 && IsDebuggerPresent()) {
        __debugbreak();
    }
#endif

    const auto* plugin = PluginDeclaration::GetSingleton();
    auto version = plugin->GetVersion();
    log::info("{} {} is loading...", plugin->GetName(), version);
    Init(skse);


    InitializeMessaging();

    //SKSE::AllocTrampoline(14 *  5);
    SKSE::AllocTrampoline(300);

    InputHook::Install();
    ReleaseHook::Install();
    SKSEInputHook::Install();
    QueueReleaseHook::Install();
    PlayerCharacter_GraphOutputEvent::Install();
    PlayerCharacter_GraphInputEvent::Install();
    
    ControlMapSaveHook::Install();
    ControlMapLoadHook::Install();
    ControlMapInitHook::Install();
    UserEventMappingCtorHook::Install();
    UserEventSaveHook::Install();


    LoadTestManager();

    log::info("{} has finished loading.", plugin->GetName());
    return true;
}
