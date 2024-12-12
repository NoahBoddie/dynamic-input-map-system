

using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

//using namespace SOS;
//using namespace RGL;

#include "TestField.h"

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



void InitializeMessaging() {
    if (!GetMessagingInterface()->RegisterListener([](MessagingInterface::Message* message) {
        switch (message->type) {
        case MessagingInterface::kPostLoad:

            break;
            // It is now safe to do multithreaded operations, or operations against other plugins.

        case MessagingInterface::kPostPostLoad: // Called after all kPostLoad message handlers have run.

            break;

        case MessagingInterface::kDataLoaded:

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
    
        SKSE::AllocTrampoline(14);

        func = SKSE::GetTrampoline().write_call<5>(hook.address(), thunk);
        
        logger::debug("hook success.");
    }


    
    static void thunk(RE::PlayerControls* a_controls, RE::InputEvent* a_event)
    {
        std::unique_ptr<RE::InputEvent> out;


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


SKSEPluginLoad(const LoadInterface* skse) {

    InitializeLogging();
#ifdef _DEBUG

    

    if (GetKeyState(VK_RCONTROL) & 0x800) {
        constexpr auto text1 = L"Request for debugger detected. If you wish to attach one and press Ok, do so now if not please press Cancel.";
        constexpr auto text2 = L"Debugger still not detected. If you wish to continue without one please press Cancel.";
        constexpr auto caption = L"Debugger Required";

        int input = 0;

        do
        {
            input = MessageBox(NULL, !input ? text1 : text2, caption, MB_OKCANCEL);
        } while (!IsDebuggerPresent() && input != IDCANCEL);
    }
#endif

    const auto* plugin = PluginDeclaration::GetSingleton();
    auto version = plugin->GetVersion();
    log::info("{} {} is loading...", plugin->GetName(), version);
    Init(skse);


    InitializeMessaging();

    InputHook::Install();

    log::info("{} has finished loading.", plugin->GetName());
    return true;
}
