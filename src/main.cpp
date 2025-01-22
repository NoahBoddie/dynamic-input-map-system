

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


struct MoveTracker
{
    inline static uint32_t timestampPrev = 0;
    inline static uint32_t timestamp = 0;

    //This system seems neat, but it only works ONCE, and second reads on the same frame
    static bool WasMoveHandled()
    {
        auto this_frame = RE::GetDurationOfApplicationRunTime();

        auto prev = timestampPrev;
        if (RE::GetDurationOfApplicationRunTime() == timestamp)
            timestampPrev = timestamp;
        else
            timestampPrev = 0;
        //1 2
        //This ensures previous only counts for the first time, but what about the second time.
        return prev && prev != this_frame;

    }

    static void MoveHandled()
    {
        auto buff = RE::GetDurationOfApplicationRunTime();
        if (buff != timestamp) {
            timestampPrev = timestamp;
            timestamp = buff;
        }
        
    }

    static void Clear()
    {
        //Ideally, this actually shouldn't be needed. I need to find a good place where a string is loaded. I think
        // C11600+2DB is a good place. Or where the string is actually set. This is in main so it's really reliable if you can sink in.
        // also, unlikely to be inlined.
        timestamp = 0;
        timestampPrev = 0;
    }
};


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


        //RE::BSInputEventQueue;

        if (a_event->AsIDEvent()) {
            auto gamepad = RE::BSInputDeviceManager::GetSingleton()->GetGamepad();

            if (auto the = skyrim_cast<RE::BSWin32GamepadDevice*>(gamepad); a_event->AsThumbstickEvent() && the)
            {
                auto other = a_event->AsThumbstickEvent();
                logger::info("happen X:({} => {}), Y:({} => {})", the->previousRX, the->currentRX, the->previousRY, the->currentRY);
                logger::info("other X:({}), Y:({})", other->xValue, other->yValue);

           
            }

            auto the = RE::BSInputDeviceManager::GetSingleton()->GetMouse();

            if (a_event->AsMouseMoveEvent() && the)
            {
                auto other = a_event->AsMouseMoveEvent();
                logger::info("device X:({} => {}), Y:({} => {})", the->dInputPrevState.x, the->dInputNextState.x, the->dInputPrevState.y, the->dInputNextState.y);
                logger::info("event X:({}), Y:({})", other->mouseInputX, other->mouseInputY);

            }

            //The smart thing here would just be to pull the gamepad every time I want to confirm this thing is legit unfortunately. Genuinely
            // no other way. This too, is also pretty unreliable. All methods unfortunately are. If the event comes from the outside
            // there is genuinely no way to tell if we've encountered the thing before (other than to manually handle while event's get sent out.
            // The other problem is it's not really saying whether it should be moving or not. But there is a system for this I have to think.
            //BECAUSE THE TWEEN MENU DIDN'T MOVE WHEN I ENTERED WITH THE THUMBSTICK ALREADY MOVING.
            // This has to mean there's already some system in place to prevent this, thus I need to find it.
            //*it does just seem to be the tween menu though.
        }
        

        if (auto button = a_event->AsButtonEvent(); button && button->value)
        {
            switch (Hash(a_event->QUserEvent().c_str()))
            {
            case "Forward"_h:
            case "Back"_h:
            case "Strafe Right"_h:
            case "Strafe Left"_h:
                MoveTracker::MoveHandled();
                break;
            }
        }
        

        auto a_continue = DIMS::testController->HandleEvent(a_controls, a_event);

        //While none of the above is quite necessary

        
        if (a_continue){
            func(a_controls, a_event);
        }

        //If we are at the end, we want to purge anything
        if (!a_event->next)
        {
            bool prev = MoveTracker::WasMoveHandled();

            //Really, I just want to see if the control maps are enabled
            if (RE::BSInputDeviceManager::GetSingleton()->IsGamepadEnabled() == false)
            {
                auto& current = a_controls->data.moveInputVec;
                auto& previous = a_controls->data.prevMoveVec;

                //TODO:Now that I think about, this breaks the cardinal rule of accuracy and not activating buttons if they were pressed 
                // while in a menu. SO maybe, pain that it is, I should just manually check the press state of all those buttons, using them
                // as a collective. OR, I can perhaps just record whether a strafe thing has been seen this frame AND had a held duration.
                // This is the best way to do it.
                bool prev_0 = std::bit_cast<uint32_t>(previous.x) || std::bit_cast<uint32_t>(previous.y);
                
                

                if (current.x || current.y || prev_0)
                {
                    //I would very much like this to use virtual events instead, but that isn't really set up properly, nor is it what one would use yet for
                    // non-finishing events.

                    VirtualEvent move_event = VirtualEvent{}.AsThumbstick("Move", false, current.x, current.y, prev_0);
                    move_event.idCode = -1;//Code here has no chance of clash, so I'm making the id here, a value that does not exist.

                    if (move_event.stage)
                    {




                        auto clear = !DIMS::testController->HandleEvent(a_controls, &move_event);


                        if (clear) {
                            current = RE::NiPoint2{ -0.f, -0.f };
                        }
                        else {
                            //I need to do this if the current axis is move as well.
                            current = move_event.axis;

                            //if (!current.x)
                            //    current.x = -0.f;
                            //if (!current.y)
                            //    current.y = -0.f;
                        }



                    }

                }
            }
        }
      

    }

    inline static REL::Relocation<decltype(thunk)> func;
};

struct PreInput_GameplayHook
{
    static void Install()
    {
        //704DE0+3B
        uintptr_t base = REL::RelocationID{ 41259, 0 }.address();
        //740A50+18D-193 in AE, with RDI being R14
        auto hook_addr = base + 0x3B;
        auto ret_addr = base + 0x41;

        struct Code : Xbyak::CodeGenerator
        {
            Code(uintptr_t ret_addr)
            {
                mov(rdi, ptr[rdi]);
                mov(rcx, rbx);
                mov(rdx, rdi);//This actually doesn't seem to be needed.
                
                mov(rax, (uintptr_t)thunk);
                call(rax);

                mov(rax, ret_addr);
                test(rdi, rdi);
                jmp(rax);

            }
        } static code{ ret_addr };

        //Null op byte after

        SKSE::GetTrampoline().write_branch<5>(hook_addr, code.getCode());

        logger::debug("hook success.");
    }

    static void thunk(RE::PlayerControls* a_controls, RE::InputEvent* a_event)
    {
        DIMS::testController->QueueAxisRelease();
        
        if (!a_event) {
            //TODO: make a dedicated release function for axis stuff pls
            DIMS::testController->ReleaseAxis(a_controls);
        }

    }

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
        MoveTracker::Clear();
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


struct PlayerCharacter_GraphVarChangedInt
{
    //This doesn't yet have the bones to get working quite yet. Needs write calls rather than virtual writes.
    /*
    static void Install()
    {
        REL::Relocation<uintptr_t> Player_GraphManager_Vtbl{ RE::VTABLE_PlayerCharacter[3] };

        func = Player_GraphManager_Vtbl.write_vfunc(0x11, thunk);
    }

    static bool thunk(RE::IAnimationGraphManagerHolder* a_this, std::int32_t& value)
    {

        auto result = func(a_this, value);

        if (result)
            testController->stateMap.Update(RefreshCode::GraphInputEvent, a_name);

        return result;
    }

    inline static REL::Relocation<decltype(thunk)> func;
    //*/
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
    //PreInput_GameplayHook::Install();//We don't need to do this, thumb and mouse events have ends.
    ReleaseHook::Install();
    SKSEInputHook::Install();
    QueueReleaseHook::Install();
    PlayerCharacter_GraphOutputEvent::Install();
    PlayerCharacter_GraphInputEvent::Install();
    
    ControlMapSaveHook::Install();
    ControlMapLoadHook::Install();
    ControlMapInitHook::Install();
    //UserEventMappingCtorHook::Install();
    UserEventSaveHook::Install();
    UserEventCategoryHook::Install();
    UserEventCategory1Hook::Install();

    LoadTestManager();

    log::info("{} has finished loading.", plugin->GetName());
    return true;
}



namespace RE
{
    //TODO: This is for this project only, just to get rid of these pieces of trash. Please relocate this to a proper place.
    InputEvent::~InputEvent() {}

    bool                 InputEvent::HasIDCode() const { return false; }
    const BSFixedString& InputEvent::QUserEvent() const { return ""; }

    IDEvent::~IDEvent() {}

    bool                 IDEvent::HasIDCode() const { return true; }
    const BSFixedString& IDEvent::QUserEvent() const { return userEvent; }
}