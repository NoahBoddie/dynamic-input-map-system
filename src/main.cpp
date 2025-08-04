

using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

//using namespace SOS;
//using namespace RGL;



#include "Hooks.h"

#include "xbyak/xbyak.h"

#include "nlohmann/json.hpp"

using namespace DIMS;


DEFAULT_LOGGER()
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




std::vector<std::pair<RE::BSFixedString, int8_t>> activeMenus;

RE::BSFixedString GetTopMenuName(int8_t depth_limit = 6)
{
    for (auto& [name, depth] : activeMenus)
    {
        if (depth < depth_limit) {
            return name;
        }
    }

    return {};
}

struct MenuOpenCloseHandler : RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
    static auto GetSingleton()
    {
        static MenuOpenCloseHandler singleton;

        return &singleton;
    }



    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* src) override
    {
        //return RE::BSEventNotifyControl::kContinue;

        static RE::UI* ui = RE::UI::GetSingleton();


        //auto menu = ui->GetMenu(event->menuName);

        logger::info("new menu: {}, opening: {}", event->menuName.c_str(), event->opening);




        for (auto& [str, entry] : ui->menuMap)
        {
            logger::info("menu: {}, opened: {}, where: {}, {:X}", str.c_str(), !!entry.menu, !!entry.menu ? entry.menu->depthPriority : -1, (uintptr_t)entry.menu.get());
        }
        /*
        if (event->opening)
        {
            auto it = ui->menuStack.back().get();
            logger::info("look: {:X}", (uintptr_t)it);
            activeMenus.push_back(std::make_pair(event->menuName, it->depthPriority));
        }
        else
        {
            auto it = activeMenus.begin();
            auto end = activeMenus.end();
            it = std::find_if(it, end, [=](auto& pair) { return pair.first == event->menuName; });

            if (end != it)
                activeMenus.erase(it);

        }
        //*/
        RE::IMenu* menu1 = nullptr;
        RE::IMenu* menu2 = nullptr;

        //RE::GetTopMostMenu(ui, menu1, 13);
        //RE::GetTopMostMenu(ui, menu2, 6);

        logger::info("Tell {} : {:X} {:X}", GetTopMenuName().c_str(), (uintptr_t)menu1, (uintptr_t)menu2);

        return RE::BSEventNotifyControl::kContinue;
    }
};


struct MenuModeChangeHandler : RE::BSTEventSink<RE::MenuModeChangeEvent>
{
    static auto GetSingleton()
    {
        static MenuModeChangeHandler singleton;

        return &singleton;
    }



    RE::BSEventNotifyControl ProcessEvent(const RE::MenuModeChangeEvent* event, RE::BSTEventSource<RE::MenuModeChangeEvent>* src) override
    {
        return RE::BSEventNotifyControl::kContinue;

        static RE::UI* ui = RE::UI::GetSingleton();


        auto menu_ = ui->GetMenu(event->menu).get();

        logger::info("new menu: {}, state: {}, where: {:X}", event->menu.c_str(), magic_enum::enum_name(event->mode.get()), (uintptr_t)menu_);


        //RE::IMenu* menu1 = nullptr;
        //RE::IMenu* menu2 = nullptr;

        //RE::GetTopMostMenu(ui, menu1, 13);
        //RE::GetTopMostMenu(ui, menu2, 6);

        //logger::info("Tell {} : {:X} {:X}", GetTopMenuName().c_str(), (uintptr_t)menu1, (uintptr_t)menu2);

        return RE::BSEventNotifyControl::kContinue;
    }
};



void InitializeMessaging() {
    if (!GetMessagingInterface()->RegisterListener([](MessagingInterface::Message* message) {
        switch (message->type) {
        case MessagingInterface::kInputLoaded: {
            RE::UI* ui = RE::UI::GetSingleton();

            //ui->AddEventSink(MenuOpenCloseHandler::GetSingleton());
            ui->AddEventSink(MenuModeChangeHandler::GetSingleton());

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
            //reorder is no longer needed for controls
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
            DIMS::testController->ReleaseAxis();
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
        DIMS::testController->HandleRelease();
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




//MenuMode controls

RE::BSFixedString GetNameFromMenu(RE::IMenu* menu)
{

    static RE::UI* ui = RE::UI::GetSingleton();

    for (auto& [name, entry] : ui->menuMap)
    {
        if (entry.menu.get() == menu) {
            return name;
        }
    }

    return "nameless";
}

struct MenuMode_InputHook
{
    static void Install()
    {
        //SE: 8A7CA0
        REL::Relocation<uintptr_t> hook{ REL::RelocationID{ 51356, 0 }, 0xC9 };

        func = SKSE::GetTrampoline().write_call<5>(hook.address(), thunk);

        logger::debug("hook success.");
    }

    static void thunk(RE::BSTArray<RE::MenuEventHandler*>& handlers, RE::InputEvent*& event)
    {
        /*
        static RE::UI* ui = RE::UI::GetSingleton();

        RE::IMenu* menu1 = nullptr;
        RE::IMenu* menu2 = nullptr;

        RE::GetTopMostMenu(ui, menu1, 14);
        RE::GetTopMostMenu(ui, menu2, 6);

        logger::info("Tell : {:X} {:X}, {}, {}, {}", (uintptr_t)menu1, (uintptr_t)menu2, GetNameFromMenu(menu1).c_str(), GetNameFromMenu(menu2).c_str(),
            RE::InterfaceStrings::GetSingleton()->topMenu.c_str());
        
        for (auto& [str, entry] : ui->menuMap)
        {
            logger::info("-menu: {}, opened: {}, where: {}, {:X}", str.c_str(), !!entry.menu, !!entry.menu ? entry.menu->depthPriority : -1, (uintptr_t)entry.menu.get());
        }
        //*/
        RE::MenuControls* controls = adjust_pointer<RE::MenuControls>(&handlers, -offsetof(RE::MenuControls, handlers));


        auto a_continue = true;//DIMS::menuController->HandleEvent(controls, event);

        //While none of the above is quite necessary


        if (a_continue) {
            func(handlers, event);
        }

        //I used to do something here, but no longer. Please be aware of this.


    }

    inline static REL::Relocation<decltype(thunk)> func;
};

struct MenuControlHandle_Hook
{
    static void Install()
    {
        REL::Relocation vtable{ RE::VTABLE_MenuControls[0] };
        RE::MenuControls;


        func = vtable.write_vfunc(0x1, thunk);

        logger::debug("hook success.");
    }

    static RE::BSEventNotifyControl thunk(RE::MenuControls* a_this, RE::InputEvent*& evt, RE::BSTEventSource<RE::InputEvent*>& src)
    {
        //DIMS::menuController->QueueRelease();

        auto result = func(a_this, evt, src);
        //I'm worried about this entering remap mode after.
        //DIMS::menuController->HandleRelease();

        return result;
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
        testController->stateHandler.Update(RefreshCode::GraphOutputEvent, a_event->tag);
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
            testController->stateHandler.Update(RefreshCode::GraphInputEvent, a_name);

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
    //InitializeLogging();
    
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
    
    MenuMode_InputHook::Install();
    MenuControlHandle_Hook::Install();

    PlayerCharacter_GraphOutputEvent::Install();
    PlayerCharacter_GraphInputEvent::Install();
    
    ControlMapSaveHook::Install();
    ControlMapLoadHook::Install();
    ControlMapInitHook::Install();
    UserEventSaveHook::Install();
    UserEventCategory_CompareHook::Install();
    UserEventCategory_IteratorHook::Install();
    UserEventCategory_TempMappingHook::Install();
    RE::TESImageSpaceModifier;
    
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



namespace DIMS::Test
{
    using namespace nlohmann;
    using json = nlohmann::json;

    using json_type = json::value_t;
    /*

    struct json_expectation
    {
        //Retain this, I'll still need it for fine tuning stuff like args.

        std::string_view name;          //The name of the entry
        json_type type;                 //The main type of the entry
        std::set<json_type> typeOf;     //If is an array, the types the array is to expect

        bool IsOptional()
        {
            return name.size() && name.front() == '~';
        }

        std::string_view GetName()
        {
            return IsOptional() ? name.substr(1) : name;
        }
    
        //Want this to give the reason so I can use it while I throw
        bool Matches(json& value)
        {
            if (value.type() != json_type::object) {
                return false;
            }
            
            bool optional = IsOptional();

            auto name = GetName();

            auto it = value.find(name);

            if (value.end() != it) {
                return optional;
            }

            if (it->type() == type) {
                return false;
            }

            if (typeOf.empty() == false)
            {
                for (auto& entry : value)
                {
                    if (typeOf.contains(entry.type()) == false) {
                        return false;
                    }
                }
            }
            
            

            return true;
        }


        json_expectation operator+(json_type type)
        {
            assert(type == json_type::array);
            auto self = *this;
            self.typeOf.insert(type);
            return self;
        }
        
        json_expectation() = default;
        json_expectation(const std::string_view& a_name, json_type a_type) : name{ a_name }, type{ a_type }
        {

        }

    };

    


    struct json_entry
    {
        json_entry(json& d) : data{ d } {}

        json* operator->()
        {
            return &data;
        }
        
        json& operator*()
        {
            return data;
        }

        operator json&()
        {
            return data;
        }



        //I may actually go with references and uses exceptions to break out of parsing.
        json& data;


        json_entry GetEntry(std::string_view name, std::span<json_expectation> expect)
        {
            auto it = data.find(name);

            if (data.end() == it)
                throw 1;

            json_entry value = it.value();

            if (value->type() != json_type::object) {
                throw 1;
            }

            std::set<std::string_view> names;

            for (auto& entry : expect)
            {
                if (entry.Matches(value) == false)
                    throw 1;

                names.insert(entry.GetName());
            }
            


            for (auto i = value->begin(); value->end() != i; i++)
            {
                if (names.contains(i.key()) == false)
                    throw 1;
            }


            return value;
        }

        json_entry GetEntry(std::string_view name, json_type type)
        {

            auto it = data.find(name);

            if (data.end() == it)
                throw 1;

            json_entry value = it.value();

            if (value->type() != type) {
                throw 1;
            }

            return value;
        }


    };
    
    json_expectation operator ""_obj(const char* data, size_t length)
    {
        return { std::string_view{ data, length }, json_type::object };
    }

    json_expectation operator ""_arr(const char* data, size_t length)
    {
        return { std::string_view{ data, length }, json_type::array };
    }

    json_expectation operator ""_str(const char* data, size_t length)
    {
        return { std::string_view{ data, length }, json_type::string };
    }

    json_expectation operator ""_bool(const char* data, size_t length)
    {
        return { std::string_view{ data, length }, json_type::boolean };
    }

    json_expectation operator ""_int(const char* data, size_t length)
    {
        return { std::string_view{ data, length }, json_type::number_integer };
    }

    json_expectation operator ""_uint(const char* data, size_t length)
    {
        return { std::string_view{ data, length }, json_type::number_unsigned };
    }

    json_expectation operator ""_flo(const char* data, size_t length)
    {
        return { std::string_view{ data, length }, json_type::number_float };
    }

    inline void Fish()
    {
        using namespace literals::json_literals;
        ""_json;
        std::ifstream f("example.json");
        json data = json::parse(f);


        auto it = data.find("string");

        data.end() == it;
        json& test  = data["interfer"];
        json type = test.type();
        json::value_t::array;
    }

    struct ValueEntry;
    struct MemberEntry;

    enum struct OptionalState : uint8_t
    {
        None,
        Optional,       //Doesn't have to exist
        LinkOptional,   // if one exists the other must
        RestOptional,   //Only one has to exist
    };

    ENUM(EntryFlag, uint8_t)
    {
        None = 0 << 0, 
        Alternatives = 1 << 0,
    };

    //An expected entry, as no name cares only about type
    struct ValueEntry
    {
        static constexpr json_type pointer_type = (json_type)-1;

        json_type type = json_type::discarded;//the type null represents alternatives
        //This is here because it's free space. It's only supposed to be used by members.
        
        EntryFlag flags{};
        OptionalState optState;
        uint32_t optCode = 0;// If -1, optional exclusively, if not and isn't 0 it's optional inclusively
        //The above will likely represent a code instead. below it will have to represent what it means.
        union
        {
            uintptr_t raw = 0;
            std::vector<ValueEntry>* _values;
            std::vector<MemberEntry>* _members;

            ValueEntry* _valuePtr;
        };

        bool IsArray()
        {
            return json_type::array == type;
        }

        bool IsObject()
        {
            return json_type::object == type;
        }

        bool IsAlternative() const
        {
            return flags & EntryFlag::Alternatives;
        }

        std::vector<ValueEntry>* GetValues()
        {
            
            return IsArray() ? _values : nullptr;
        }

        std::vector<MemberEntry>* GetMembers()
        {

            return IsObject() ? _members : nullptr;
        }




        //std::set<uint32_t> activeCodes;Id like to add this into matches
        bool Matches(json& value, std::set<uint64_t>& activeCodes)
        {
            //Active code is a mix of the optional state and the hash. The optional state is added onto the string and then used like that.
            bool matches = value.type() == type;
            switch (type)
            {
            case json_type::array:
                if (auto values = GetValues())
                {
                    bool alt = IsAlternative();
                    //I'd like to rewrite this, it's kinda weird as shit.
                    if (!alt && !matches)
                        return false;
    
                    auto size = alt ? 1 : value.size();


                    for (int i = 0; i < size; i++)
                    {
                        auto& x = alt ? value : value[i];

                        bool result = false;

                        for (auto& y : *values) {
                            result = y.Matches(x);

                            if (result)
                                break;
                        }

                        if (!result)
                            return false;
                    }

                    return alt || matches;
                }
                break;

            case json_type::object:
                if (auto members = GetMembers())
                {
                    std::set<std::string_view> names;

                    for (auto& entry : *members)
                    {
                        if (entry.Matches(value) == false)
                            return false;

                        names.insert(entry.GetName());
                    }



                    for (auto i = value.begin(); value.end() != i; i++)
                    {
                        if (names.contains(i.key()) == false)
                            return false;
                    }

                }

                break;

            
            case json_type::number_unsigned:
            case json_type::number_integer:
            case json_type::number_float:
            case json_type::boolean:
            case json_type::string:
               // return matches;
                break;

            case pointer_type:
                return _valuePtr ? _valuePtr->Matches(value, activeCodes) : false;

            default:
                throw nullptr;
            }



            return matches;
        }

        bool Matches(json& value)
        {
            std::set<uint64_t> activeCodes;

            return Matches(value, activeCodes);
        }


        ~ValueEntry()
        {
            if (!raw)
                return;

            switch (type)
            {
            case json_type::array:
                delete _values;
                break;

            case json_type::object:
                delete _members;
                break;

            default:
                throw nullptr;
            }
        }
    };

    struct MemberEntry : public ValueEntry
    {

        std::string_view name;
        

        std::string_view GetName()
        {
            return name;
        }

        bool IsOptional()
        {
            return false;
        }

        bool Matches(json& value, std::set<uint64_t>& activeCodes)
        {
            if (value.is_object() == false) {
                return IsOptional();
            }
            auto it = value.find(name);

            if (value.end() == it)
                return IsOptional();

            return __super::Matches(it.value(), activeCodes);
        }

        bool Matches(json& value)
        {
            return __super::Matches(value);
        }
    };



    struct json_expectation2
    {
        std::string_view name;          //The name of the entry
        json_type type;                 //The main type of the entry
        std::set<json_type> typeOf;     //If is an array, the types the array is to expect

        bool IsOptional()
        {
            return name.size() && name.front() == '~';
        }

        std::string_view GetName()
        {
            return IsOptional() ? name.substr(1) : name;
        }

        //Want this to give the reason so I can use it while I throw
        bool Matches(json& value)
        {
            if (value.type() != json_type::object) {
                return false;
            }

            bool optional = IsOptional();

            auto name = GetName();

            auto it = value.find(name);

            if (value.end() != it) {
                return optional;
            }

            if (it->type() == type) {
                return false;
            }

            if (typeOf.empty() == false)
            {
                for (auto& entry : value)
                {
                    if (typeOf.contains(entry.type()) == false) {
                        return false;
                    }
                }
            }



            return true;
        }
    };
    //*/
    void VisitJsonArray(json& value, std::function<void(json&)> func)
    {
        bool is_array = value.is_array();

        auto size = is_array ? 1 : value.size();


        for (int i = 0; i < size; i++)
        {
            auto& entry = is_array ? value[i] : value;

            func(entry);
        }
    }

    RE::InputContextID GetContextFromString(const std::string_view& str)
    {
        switch (Hash<HashFlags::Insensitive>(str))
        {
            /*
            kGameplay = 0,
                kMenuMode,
                kConsole,
                kItemMenu,
                kInventory,
                kDebugText,
                kFavorites,
                kMap,
                kStats,
                kCursor,
                kBook,
                kDebugOverlay,
                kJournal,
                kTFCMode,
                kMapDebug,
                kLockpicking,

                kFavor,

                kTotal,
            //*/

        case "Gameplay"_ih: return RE::InputContextID::kGameplay;
        case "MenuMode"_ih: return RE::InputContextID::kMenuMode;
        case "Console"_ih: return RE::InputContextID::kConsole;
        case "ItemMenu"_ih: return RE::InputContextID::kItemMenu;
        case "Inventory"_ih: return RE::InputContextID::kInventory;
        case "Favorites"_ih: return RE::InputContextID::kFavorites;
        case "Map"_ih: return RE::InputContextID::kMap;
        case "Stats"_ih: return RE::InputContextID::kStats;
        case "Cursor"_ih: return RE::InputContextID::kCursor;
        case "Book"_ih: return RE::InputContextID::kBook;
        case "DebugOverlay"_ih: return RE::InputContextID::kDebugOverlay;
        case "Journal"_ih: return RE::InputContextID::kJournal;
        case "MapDebug"_ih: return RE::InputContextID::kMapDebug;
        case "Lockpicking"_ih: return RE::InputContextID::kLockpicking;
        case "Favor"_ih: return RE::InputContextID::kFavor;
        case "FreeCamera"_ih: return RE::InputContextID::kTFCMode;
        case "DebugText"_ih: return RE::InputContextID::kDebugText;//I will likely refer to this by a different name. 
        default: return RE::InputContextID::kNone;

        }
    }

    RE::UserEventFlag GetUseFlagFromString(const std::string_view& str)
    {
        switch (Hash<HashFlags::Insensitive>(str))
        {
        case "kMovement"_ih: return RE::UserEventFlag::kMovement;
        case "kLooking"_ih: return RE::UserEventFlag::kLooking;
        case "kActivate"_ih: return RE::UserEventFlag::kActivate;
        case "kPOVSwitch"_ih: return RE::UserEventFlag::kPOVSwitch;
        case "kFighting"_ih: return RE::UserEventFlag::kFighting;
        case "kSneaking"_ih: return RE::UserEventFlag::kSneaking;
        case "kMainFour"_ih: return RE::UserEventFlag::kMainFour;
        case "kWheelZoom"_ih: return RE::UserEventFlag::kWheelZoom;
        case "kJumping"_ih: return RE::UserEventFlag::kJumping;
        case "kVATS"_ih: return RE::UserEventFlag::kVATS;

        default: return RE::UserEventFlag::kNone;
        }
    }


    constexpr uint16_t nullKey = 255;

    //Remember for all of these to also use the skse dx codes. This will make it easier to input even if you dont have the name.
    // Still string though.
    uint16_t GetGamepadFromString(const std::string_view& str)
    {
        switch (Hash<HashFlags::Insensitive>(str))
        {

        case "0x10A"_ih:
        case "266"_ih:
        case "Up"_ih:
            return RE::GamepadInput::XINPUT_GAMEPAD_DPAD_UP;

        case "0x10B"_ih:
        case "267"_ih:
        case "Down"_ih:
            return RE::GamepadInput::XINPUT_GAMEPAD_DPAD_DOWN;

        case "0x10C"_ih:
        case "268"_ih:
        case "Left"_ih:
            return RE::GamepadInput::XINPUT_GAMEPAD_DPAD_LEFT;

        case "0x10D"_ih:
        case "269"_ih:
        case "Right"_ih:
            return RE::GamepadInput::XINPUT_GAMEPAD_DPAD_RIGHT;



        case "0x10E"_ih:
        case "270"_ih:
        case "Start"_ih:
            return RE::GamepadInput::XINPUT_GAMEPAD_START;


        case "0x10F"_ih:
        case "271"_ih:
        case "Back"_ih:
            return RE::GamepadInput::XINPUT_GAMEPAD_BACK;

        //case "L3"_ih://Alias
        case "0x110"_ih:
        case "272"_ih:
        case "Left Thumb"_ih:
        
            return RE::GamepadInput::XINPUT_GAMEPAD_LEFT_THUMB;




        //case "R3"_ih://Alias
        case "0x111"_ih:
        case "273"_ih:
        case "Right Thumb"_ih:
            return RE::GamepadInput::XINPUT_GAMEPAD_RIGHT_THUMB;


        case "RS"_ih:
        case "Right Stick"_ih:
            return 0xB;


        case "LS"_ih:
        case "Left Stick"_ih:
            return 0xC;



        case "0x112"_ih:
        case "274"_ih:
        case "Left Shoulder"_ih:
            return RE::GamepadInput::XINPUT_GAMEPAD_LEFT_SHOULDER;


        case "0x113"_ih:
        case "275"_ih:
        case "Right Shoulder"_ih:
            return RE::GamepadInput::XINPUT_GAMEPAD_RIGHT_SHOULDER;



        case "0x118"_ih:
        case "280"_ih:
        case "Left Trigger"_ih:
            return 0x9;

        case "0x119"_ih:
        case "281"_ih:
        case "Right Trigger"_ih:
            return 0xA;


        //Cardinals can be used for this as well.
              
        case "0x117"_ih:
        case "279"_ih:
        case "Top Face"_ih:
            return RE::GamepadInput::XINPUT_GAMEPAD_Y;



        case "0x114"_ih:
        case "276"_ih:
        case "Bottom Face"_ih:
            return RE::GamepadInput::XINPUT_GAMEPAD_A;



        case "0x116"_ih:
        case "278"_ih:
        case "Left Face"_ih:
            return RE::GamepadInput::XINPUT_GAMEPAD_X;


        case "0x115"_ih:
        case "277"_ih:
        case "Right Face"_ih:
            return RE::GamepadInput::XINPUT_GAMEPAD_B;

        default:
            return nullKey;
        }

    }

    uint16_t GetKeyboardFromString(const std::string_view& str)
    {
        switch (Hash<HashFlags::Insensitive>(str))
        {
  
        case "0x01"_ih:
        case "Escape"_ih:
            return RE::Keyboard::DIK_ESCAPE;

        case "0x02"_ih:
        case "1"_ih:
            return RE::Keyboard::DIK_1;

        case "0x03"_ih:
        case "2"_ih:
            return RE::Keyboard::DIK_2;

        case "0x04"_ih:
        case "3"_ih:
            return RE::Keyboard::DIK_3;

        case "0x05"_ih:
        case "4"_ih:
            return RE::Keyboard::DIK_4;

        case "0x06"_ih:
        case "5"_ih:
            return RE::Keyboard::DIK_5;

        case "0x07"_ih:
        case "6"_ih:
            return RE::Keyboard::DIK_6;

        case "0x08"_ih:
        case "7"_ih:
            return RE::Keyboard::DIK_7;

        case "0x09"_ih:
        case "8"_ih:
            return RE::Keyboard::DIK_8;

        case "0x0A"_ih:
        case "9"_ih:
            return RE::Keyboard::DIK_9;

        case "0x0B"_ih:
        case "0"_ih:
            return RE::Keyboard::DIK_0;

        case "-"_ih:
        case "0x0C"_ih:
        case "Minus"_ih:
            return RE::Keyboard::DIK_MINUS;

        case "="_ih:
        case "0x0D"_ih:
        case "Equals"_ih:
            return RE::Keyboard::DIK_EQUALS;

        case "0x0E"_ih:
        case "Backspace"_ih:
            return RE::Keyboard::DIK_BACKSPACE;

        case "0x0F"_ih:
        case "Tab"_ih:
            return RE::Keyboard::DIK_TAB;

        case "0x10"_ih:
        case "Q"_ih:
            return RE::Keyboard::DIK_Q;

        case "0x11"_ih:
        case "W"_ih:
            return RE::Keyboard::DIK_W;

        case "0x12"_ih:
        case "E"_ih:
            return RE::Keyboard::DIK_E;

        case "0x13"_ih:
        case "R"_ih:
            return RE::Keyboard::DIK_R;

        case "0x14"_ih:
        case "T"_ih:
            return RE::Keyboard::DIK_T;

        case "0x15"_ih:
        case "Y"_ih:
            return RE::Keyboard::DIK_Y;

        case "0x16"_ih:
        case "U"_ih:
            return RE::Keyboard::DIK_U;

        case "0x17"_ih:
        case "I"_ih:
            return RE::Keyboard::DIK_I;

        case "0x18"_ih:
        case "O"_ih:
            return RE::Keyboard::DIK_O;

        case "0x19"_ih:
        case "P"_ih:
            return RE::Keyboard::DIK_P;

        case "["_ih:
        case "0x1A"_ih:
        case "Left Bracket"_ih:
            return RE::Keyboard::DIK_LBRACKET;

        case "]"_ih:
        case "0x1B"_ih:
        case "Right Bracket"_ih:
            return RE::Keyboard::DIK_RBRACKET;

        case "0x1C"_ih:
        case "Enter"_ih:
        case "Return"_ih:
            return RE::Keyboard::DIK_RETURN;

        case "0x1D"_ih:
        case "Left Control"_ih:
            return RE::Keyboard::DIK_LCONTROL;

        case "0x1E"_ih:
        case "A"_ih:
            return RE::Keyboard::DIK_A;

        case "0x1F"_ih:
        case "S"_ih:
            return RE::Keyboard::DIK_S;

        case "0x20"_ih:
        case "D"_ih:
            return RE::Keyboard::DIK_D;

        case "0x21"_ih:
        case "F"_ih:
            return RE::Keyboard::DIK_F;

        case "0x22"_ih:
        case "G"_ih:
            return RE::Keyboard::DIK_G;

        case "0x23"_ih:
        case "H"_ih:
            return RE::Keyboard::DIK_H;

        case "0x24"_ih:
        case "J"_ih:
            return RE::Keyboard::DIK_J;

        case "0x25"_ih:
        case "K"_ih:
            return RE::Keyboard::DIK_K;

        case "0x26"_ih:
        case "L"_ih:
            return RE::Keyboard::DIK_L;

        case "0x27"_ih:
        case ":"_ih:
        case "Semicolon"_ih:
            return RE::Keyboard::DIK_SEMICOLON;

        case "0x28"_ih:
        case "'"_ih:
        case "Apostrophe"_ih:
            return RE::Keyboard::DIK_APOSTROPHE;

        case "0x29"_ih:
        case "`"_ih:
        case "Grave"_ih:
            return RE::Keyboard::DIK_GRAVE;

        case "0x2A"_ih:
        case "Left Shift"_ih:
            return RE::Keyboard::DIK_LSHIFT;

        case "\\"_ih:
        case "0x2B"_ih:
        case "Back Slash"_ih:
            return RE::Keyboard::DIK_BACKSLASH;

        case "0x2C"_ih:
        case "Z"_ih:
            return RE::Keyboard::DIK_Z;

        case "0x2D"_ih:
        case "X"_ih:
            return RE::Keyboard::DIK_X;

        case "0x2E"_ih:
        case "C"_ih:
            return RE::Keyboard::DIK_C;

        case "0x2F"_ih:
        case "V"_ih:
            return RE::Keyboard::DIK_V;

        case "0x30"_ih:
        case "B"_ih:
            return RE::Keyboard::DIK_B;

        case "0x31"_ih:
        case "N"_ih:
            return RE::Keyboard::DIK_N;

        case "0x32"_ih:
        case "M"_ih:
            return RE::Keyboard::DIK_M;

        case ","_ih:
        case "0x33"_ih:
        case "Comma"_ih:
            return RE::Keyboard::DIK_COMMA;

        case "."_ih:
        case "0x34"_ih:
        case "Period"_ih:
            return RE::Keyboard::DIK_PERIOD;

        case "/"_ih:
        case "0x35"_ih:
        case "Forward Slash"_ih:
            return RE::Keyboard::DIK_SLASH;

        case "0x36"_ih:
        case "Right Shift"_ih:
            return RE::Keyboard::DIK_RSHIFT;

        case "0x37"_ih:
        case "*"_ih:
        case "NUM*"_ih:
            return RE::Keyboard::DIK_MULTIPLY;

        case "0x38"_ih:
        case "Left Alt"_ih:
            return RE::Keyboard::DIK_LALT;

        case "0x39"_ih:
        case "Spacebar"_ih:
            return RE::Keyboard::DIK_SPACE;

        case "0x3A"_ih:
        case "Caps Lock"_ih:
            return RE::Keyboard::DIK_CAPSLOCK;

        case "0x3B"_ih:
        case "F1"_ih:
            return RE::Keyboard::DIK_F1;

        case "0x3C"_ih:
        case "F2"_ih:
            return RE::Keyboard::DIK_F2;

        case "0x3D"_ih:
        case "F3"_ih:
            return RE::Keyboard::DIK_F3;

        case "0x3E"_ih:
        case "F4"_ih:
            return RE::Keyboard::DIK_F4;

        case "0x3F"_ih:
        case "F5"_ih:
            return RE::Keyboard::DIK_F5;

        case "0x40"_ih:
        case "F6"_ih:
            return RE::Keyboard::DIK_F6;

        case "0x41"_ih:
        case "F7"_ih:
            return RE::Keyboard::DIK_F7;

        case "0x42"_ih:
        case "F8"_ih:
            return RE::Keyboard::DIK_F8;

        case "0x43"_ih:
        case "F9"_ih:
            return RE::Keyboard::DIK_F9;

        case "0x44"_ih:
        case "F10"_ih:
            return RE::Keyboard::DIK_F10;

        case "0x45"_ih:
        case "Num Lock"_ih:
            return RE::Keyboard::DIK_NUMLOCK;

        case "0x46"_ih:
        case "Scroll Lock"_ih:
            return RE::Keyboard::DIK_SCROLL;

        case "0x47"_ih:
        case "NUM7"_ih:
            return RE::Keyboard::DIK_NUMPAD7;

        case "0x48"_ih:
        case "NUM8"_ih:
            return RE::Keyboard::DIK_NUMPAD8;

        case "0x49"_ih:
        case "NUM9"_ih:
            return RE::Keyboard::DIK_NUMPAD9;

        case "0x4A"_ih:
        case "NUM-"_ih:
            return RE::Keyboard::DIK_NUMPADMINUS;

        case "0x4B"_ih:
        case "NUM4"_ih:
            return RE::Keyboard::DIK_NUMPAD4;

        case "0x4C"_ih:
        case "NUM5"_ih:
            return RE::Keyboard::DIK_NUMPAD5;

        case "0x4D"_ih:
        case "NUM6"_ih:
            return RE::Keyboard::DIK_NUMPAD6;

        case "0x4E"_ih:
        case "NUM+"_ih:
            return RE::Keyboard::DIK_NUMPADPLUS;

        case "0x4F"_ih:
        case "NUM1"_ih:
            return RE::Keyboard::DIK_NUMPAD1;

        case "0x50"_ih:
        case "NUM2"_ih:
            return RE::Keyboard::DIK_NUMPAD2;

        case "0x51"_ih:
        case "NUM3"_ih:
            return RE::Keyboard::DIK_ESCAPE;

        case "0x52"_ih:
        case "NUM0"_ih:
            return RE::Keyboard::DIK_NUMPAD0;

        case "0x53"_ih:
        case "NUM."_ih:
            return RE::Keyboard::DIK_NUMPADPERIOD;

        case "0x57"_ih:
        case "F11"_ih:
            return RE::Keyboard::DIK_F11;

        case "0x58"_ih:
        case "F12"_ih:
            return RE::Keyboard::DIK_F12;

        case "0x9C"_ih:
        case "NUM Return"_ih:
        case "NUM Enter"_ih:
            return RE::Keyboard::DIK_NUMPADENTER;

        case "0x9D"_ih:
        case "Right Control"_ih:
            return RE::Keyboard::DIK_RCONTROL;

        case "0xB5"_ih:
        case "NUM/"_ih:
            return RE::Keyboard::DIK_NUMPADSLASH;

        case "0xB7"_ih:
        case "Print Screen"_ih:
        case "SysRq"_ih:
            return RE::Keyboard::DIK_SYSRQ;

        case "0xB8"_ih:
        case "Right Alt"_ih:
            return RE::Keyboard::DIK_RALT;

        case "0xC5"_ih:
        case "Pause"_ih:
            return RE::Keyboard::DIK_PAUSE;

        case "0xC7"_ih:
        case "Home"_ih:
            return RE::Keyboard::DIK_HOME;

        case "0xC8"_ih:
        case "Up"_ih:
        case "Up Arrow"_ih:
            return RE::Keyboard::DIK_UPARROW;

        case "0xC9"_ih:
        case "PgUp"_ih:
        case "Page Up"_ih:
            return RE::Keyboard::DIK_PGUP;

        case "0xCB"_ih:
        case "Left"_ih:
        case "Left Arrow"_ih:
            return RE::Keyboard::DIK_LEFTARROW;

        case "0xCD"_ih:
        case "Right"_ih:
        case "Right Arrow"_ih:
            return RE::Keyboard::DIK_RIGHTARROW;

        case "0xCF"_ih:
        case "End"_ih:
            return RE::Keyboard::DIK_END;

        case "0xD0"_ih:
        case "Down"_ih:
        case "Down Arrow"_ih:
            return RE::Keyboard::DIK_DOWNARROW;

        case "0xD1"_ih:
        case "PgDown"_ih:
        case "Page Down"_ih:
            return RE::Keyboard::DIK_PGDN;

        case "0xD2"_ih:
        case "Insert"_ih:
            return RE::Keyboard::DIK_INSERT;

        case "0xD3"_ih:
        case "Delete"_ih:
            return RE::Keyboard::DIK_DELETE;

        default:
            return nullKey;
        }

    }

    uint16_t GetMouseFromString(const std::string_view& str)
    {
        switch (Hash<HashFlags::Insensitive>(str))
        {
        //*
        case "0x100"_ih:
        //case "Mouse 1"_ih:
        case "Left Mouse"_ih:
        case "Left Mouse Button"_ih:
            return 0;

        case "0x101"_ih:
        //case "Mouse 2"_ih:
        case "Right Mouse"_ih:
        case "Right Mouse Button"_ih:
            return 1;

        case "0x102"_ih:
        case "Middle Mouse"_ih:
        case "Middle Mouse Button"_ih:
            return 2;
                
        case "0x103"_ih:
        case "Mouse 3"_ih:
        case "Mouse Button 3"_ih:
            return 3;
                
        case "0x104"_ih:
        case "Mouse 4"_ih:
        case "Mouse Button 4"_ih:
            return 4;
                
        case "0x105"_ih:
        case "Mouse 5"_ih:
        case "Mouse Button 5"_ih:
            return 5;
                
        case "0x106"_ih:
        case "Mouse 6"_ih:
        case "Mouse Button 6"_ih:
            return 6;
                
        case "0x107"_ih:
        case "Mouse 7"_ih:
        case "Mouse Button 7"_ih:
            return 7;
                
        case "0x108"_ih:
        case "Scroll Up"_ih:
        case "Mouse Wheel Up"_ih:
            return 8;
                
        case "0x109"_ih:
        case "Scroll Down"_ih:
        case "Mouse Wheel Down"_ih:
            return 9;
                
        case "Move"_ih:
            return 10;

        //*/

        default:
            return nullKey;
        }
    }


    uint16_t GetKeyFromString(const std::string_view& str, RE::InputDevice device)
    {
        switch (device)
        {
        case RE::InputDevice::kGamepad:
            return GetGamepadFromString(str);

        case RE::InputDevice::kKeyboard:
            return GetKeyboardFromString(str);

        case RE::InputDevice::kMouse:
            return GetMouseFromString(str);
        default:
            return nullKey;
        }


    }

    uint16_t GetKBMFromString(const std::string_view& str)
    {
        auto input = GetMouseFromString(str);

        if (input == nullKey) {
            input = GetKeyboardFromString(str);
        }

        return input;
    }

    RE::InputDevice GetDeviceFromString(const std::string_view& str)
    {
        switch (Hash<HashFlags::Insensitive>(str))
        {
        case "Gamepad"_ih:
            return RE::InputDevice::kGamepad;

        case "Keyboard"_ih:
            return RE::InputDevice::kKeyboard;

        case "Mouse"_ih:
            return RE::InputDevice::kMouse;

        default:
            return RE::InputDevice::kNone;
        }
    }

    template <typename T>
    T FindOr(json& ref, const std::string_view& name, T def)
    {
        auto it = ref.find(name);
       
        if (ref.end() == it)
            return def;
        else
            return it.value();
    }

    
    bool IfFind(json& ref, const std::string_view& name, std::function<void(json::iterator&)> func)
    {
        auto it = ref.find(name);

        auto find = ref.end() != it;
        
        if (find)
            func(it);

        return find;
    }


    bool IfFind(json& ref, const std::string_view& name, std::function<void(json&)> func)
    {
        auto it = ref.find(name);

        auto find = ref.end() != it;

        if (find)
            func(it.value());

        return find;
    }


    
    
    
    
    struct CustomControl
    {
        //When loading, the invalid device types will be ignored depending on what game type is loaded.
        using UserEvent = std::array<CustomEvent, RE::InputDevice::kTotal>;
        struct Setting
        {
            struct Key
            {
                uint16_t input = 255;
                uint16_t modifier = 255;
            };
            std::vector<Key> keys;
            
            bool remappable = false;
            
            bool IsRemappable() const
            {
                return remappable && keys.size() <= 1;
            }

            bool NeedsEntry() const
            {
                return !keys.empty() || IsRemappable();
            }

            void AddKey(uint16_t input, uint16_t modifier)
            {
                //If this is null I'd like something to handle it.

                if (input != nullKey)
                {
                    keys.emplace_back(input, modifier);
                }
            }


            void LoadFromRecord(json& setting, RE::InputDevice device)
            {
                VisitJsonArray(setting, [&](json& it) -> void {
                    uint16_t input = nullKey;
                    uint16_t modifier = nullKey;

                    switch (it.type())
                    {
                    case json_type::string:
                        input = GetKeyFromString(setting, device);
                        break;

                    case json_type::object:
                        input = GetKeyFromString(setting["input"], device);
                        IfFind(setting, "modifier", [&](json& it)
                            {
                                modifier = GetKeyFromString(setting, device);
                            });

                        IfFind(setting, "remappable", [this](json& it)
                            {
                                remappable = it;
                            });

                        break;

                    default:
                        logger::info("invalid object used, oops");
                        break;
                    }

                    AddKey(input, modifier);
                    });
                

            }

        };



         //devices;

        std::string_view name;
        std::string_view filename;
        std::string_view category;
        std::map<RE::InputContextID, std::array<Setting, RE::InputDevice::kTotal>> contexts;

        RE::UserEventFlag useFlags = RE::UserEventFlag::kNone;
        bool shouldRegister = false;


        void SendOff()
        {
            if (shouldRegister){
                CustomMapping::Create(name, filename, category);
            }

            RE::ControlMap* map = RE::ControlMap::GetSingleton();

            for (auto& [context, devices] : contexts)
            {
                auto& contextList = map->controlMap[context];

                if (!contextList)
                    continue;

                for (auto i = (RE::InputDevice)0; i < RE::InputDevice::kVirtualKeyboard; i++)
                {
                    auto& device = devices[i];

                    if (device.NeedsEntry() == true)
                    {
                        auto& mapping = contextList->deviceMappings[i];

                        for (auto [input, modifier] : device.keys){
                            mapping.push_back(CustomEvent{ name, input, modifier, useFlags, device.IsRemappable() });
                        }
                    }
                }
            }
        }

        void LoadFromRecord(const std::string_view& file, json& control)
        {

            name = control["name"];
            category = FindOr(control, "category", ""sv);

            json* focus = nullptr;

            RE::InputContextID context = RE::InputContextID::kGameplay;

            std::function<void(json&)> device_handle = [&](json& focus) {

                json& device_list = focus["devices"];

                auto& devices = contexts[context];

                for (RE::InputDevice i = (RE::InputDevice)0; i < RE::InputDevice::kVirtualKeyboard; i++)
                {
                    auto& device = devices[i];

                    if (device.IsRemappable() == true) {
                        //device has already been configured
                        continue;
                    }

                    std::string_view device_name = magic_enum::enum_name(i).substr(1);

                    IfFind(device_list, device_name, [&](json& it)
                        {
                            devices[i].LoadFromRecord(it, i);

                            if (device.IsRemappable() == true)
                            {
                                shouldRegister = true;

                                RE::InputDevice other;
                                switch (i)
                                {
                                case RE::InputDevice::kMouse:
                                    other = RE::InputDevice::kKeyboard;
                                    goto comp;
                                case RE::InputDevice::kKeyboard:
                                    other = RE::InputDevice::kMouse;
                                comp: devices[other].remappable = true;

                                    break;
                                }
                            }
                        });



         
                }

            };


            if (IfFind(control, "context", [&](json& it)
                {

                    for (int i = 0; i < it.size(); i++)
                    {
                        auto& entry = it[i];

                        context = GetContextFromString(entry["name"]);
                        if (context != RE::InputContextID::kNone)
                            device_handle(entry);
                    }
                }) == false)
            {
                device_handle(control);
            }


            IfFind(control, "flags", [&](json& it)
                {
                    VisitJsonArray(it, [this](json& it)
                        {
                            auto flag = GetUseFlagFromString(it);

                            if (!flag || !!(useFlags & flag))
                                return;//TODO: please log error

                            useFlags |= flag;
                        });

                });

        }
    };




    struct FakeInputCommand : public InputCommand
    {
        void LoadFromJson(const std::string_view& file, json& control)
        {

        }
    };



    void MakeControls(InputCommand& command)
    {

    }


    void MakeCommands(InputState& state)
    {

    }

    void MakeCommands(InputMode& mode)
    {

    }


    void Maker(std::string_view file, json& value)
    {
        
    }

}