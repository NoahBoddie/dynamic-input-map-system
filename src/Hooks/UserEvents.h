#pragma once



//Temp
#include "TestField.h"


#include "xbyak/xbyak.h"

namespace DIMS
{


#pragma pack(push,1)
    struct  UserEventData
    {
        union
        {
            struct
            {
                uint64_t fileID;
                uint32_t eventID;
                uint16_t inputID;
                uint16_t modifierID;
            };

            uint8_t bytes[16];
        };
        static constexpr size_t size()
        {
            return 16;
        }

        

        void Serialize(std::ofstream& file)
        {
            for (int i = 0; i < size(); i++)
                file << bytes[i];
        }
        
        void Serialize(std::ifstream& file)
        {
            if (file.good() == false)
                return;

            for (int i = 0; i < size(); i++)
                file >> bytes[i];
        }

        UserEventData(CustomEvent* mapping) : 
            fileID{ mapping->mapping()->file() },
            eventID{mapping->mapping()->id()},
            inputID{ mapping->inputKey },
            modifierID{mapping->modifier}
        {

        }

        UserEventData(CustomEvent& mapping) : UserEventData{ &mapping }
        {

        }

        UserEventData(std::ifstream& file)
        {
            Serialize(file);
        }
        
    };
#pragma pack (pop)



    int IsCallOrJump(uintptr_t addr)
    {
        //0x15 0xE8//These are calls, represented by negative numbers
        //0x25 0xE9//These are jumps, represented by positive numbers.
        //And zero represent it being neither.

        if (addr)
        {
            auto first_byte = reinterpret_cast<uint8_t*>(addr);

            switch (*first_byte)
            {
            case 0x15:
            case 0xE8:
                return -1;

            case 0x25:
            case 0xE9:
                return 1;

            }
        }

        return 0;
    }


    //write_call
    struct ControlMapLoadHook
    {
        //Believe it or not, this is actually a super important hook now, this is when controls get loaded into the game

        static void Install()
        {
            //C10DC0+10B
            REL::Relocation<uintptr_t> hook{ REL::RelocationID{67234, 0}, 0x10B };

            func = SKSE::GetTrampoline().write_call<5>(hook.address(), thunk);

            logger::debug("hook success.");
        }



        static void thunk(RE::ControlMap* a_this)
        {

            func(a_this);
            
            auto& control_map = a_this->controlMap[RE::InputContextID::kGameplay];

            uint8_t version;
            uint8_t total;

            //This needs to be more careful and explicit. At the end, check if it's reached it's end, throw if it hasn't

            std::ifstream myfile;
            myfile.open(controlMapPath);

            if (myfile.bad() == true)
                return;

            myfile >> version;

            if (version != controlMapVersion) {
                return;
            }

            myfile >> total;

            logger::info("V:{}, d:{} loading...", version, total);
            


            for (int device = 0; device < total; device++)
            {
                int32_t size;

                myfile >> size;

                logger::info("Device {} size = {}", magic_enum::enum_name((RE::INPUT_DEVICE)device), size);

                auto& device_list = ~control_map->deviceMappings[device];

                while (0 < size--)
                {
                    UserEventData data{ myfile };

                    logger::info("Load ( fileID:{}, eventID:{}, inputID:{}, modifier:{} )", data.fileID, data.eventID, data.inputID, data.modifierID);
                    bool success = false;

                    for (CustomEvent& entry : device_list)
                    {
                        if (entry.CanRemapTo(data.fileID, data.eventID) == true) {
                            entry.inputKey = data.inputID;
                            entry.modifier = data.modifierID;
                            success = true;
                            break;
                        }
                    }

                    if (!success) {
                        logger::warn("didn't deserialize file {} index {}", data.fileID, data.eventID);
                    }
                }
            }

            if (myfile.eof() == false) {
                //produce error.
            }
        }

        inline static REL::Relocation<decltype(thunk)> func;
    };
    
    //write_call
    struct ControlMapSaveHook
    {
        //Believe it or not, this is actually a super important hook now, this is when controls get loaded into the game

        static void Install()
        {
            //8EF500+B
            REL::Relocation<uintptr_t> hook{ REL::RelocationID{52374, 0}, 0xB };

            func = SKSE::GetTrampoline().write_call<5>(hook.address(), thunk);

            logger::debug("hook success.");
        }



        static void thunk(RE::ControlMap* a_this, const char* a_path)
        {
            func(a_this, a_path);
            
            auto& control_map = a_this->controlMap[RE::InputContextID::kGameplay];

            std::vector<UserEventData> serializeMap[RE::INPUT_DEVICE::kTotal];

            //this shit shouldn't be allowed, it should be simpler than this. Please use ClibVR, that shit is better suited for this.
            uint8_t total = REL::Module::IsVR() ? (RE::INPUT_DEVICE)9 : RE::INPUT_DEVICE::kVirtualKeyboard;


            bool saved = false;

            for (auto i = (RE::INPUT_DEVICE)0; i < total; i++)
            {
                auto& device_list = ~control_map->deviceMappings[i];
               
                for (auto& user_event : device_list)
                {
                    if (user_event.remappable && user_event.IsCustomEvent() == true)
                    {
                        auto& data = serializeMap[i].emplace_back(user_event);

                        

                        saved = true;
                    }
                }
            }


            if (saved)
            {
                logger::info("saving...");
                std::ofstream myfile;
                myfile.open(controlMapPath);

                
                

                myfile << controlMapVersion;
                myfile << total;


                for (int device = 0; device < total; device++)
                {
                    auto& list = serializeMap[device];

                    int32_t size = (int32_t)list.size();

                    myfile << size;

                    logger::info("Save ( device {} size = {} )", device, size);

                    for (auto& data : list) {
                        data.Serialize(myfile);
                        logger::info("Save ( fileID:{}, eventID:{}, inputID:{}, modifier:{} )", data.fileID, data.eventID, data.inputID, data.modifierID);
                    }
                }
            }
        }

        inline static REL::Relocation<decltype(thunk)> func;
    };



    //prologue
    struct ControlMapInitHook
    {
        static void Install()
        {
            //SE: 0xC13570, AE: 0x000000, VR: ???
            auto hook_addr = REL::RelocationID(67265, 00000).address();
            auto ret_addr = hook_addr + 0x5;
            struct Code : Xbyak::CodeGenerator
            {
                Code(uintptr_t ret_addr)
                {
                    mov(ptr[rsp - 0x8 + 0x10], rcx);
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


            logger::info("ControlMapInitHook complete...");
        }

        static void thunk(RE::ControlMap* a_this, const char* path)
        {
            func(a_this, path);

            //This is honestly the simplest way to purge the data of something so trival.
            //Ultimately this means people adding stuff in will be unsafe though.
            /*
            for (auto x = 0; x < RE::InputContextID::kTotal; x++)
            {
                for (auto i = 0; i < RE::INPUT_DEVICE::kVirtualKeyboard; i++)
                {
                    for (auto& value : a_this->controlMap[x]->deviceMappings[i])
                    {
                        value.pad14 = -1;
                    }
                }
            }
            //*/
            //return;//This is retired for now, due to not really having a proper alignment and it's name.

            CustomEvent TEST_B{ "TEST_B", "TestDocument", "TESTAREA", 1, true};

            TEST_B.inputKey = 48;
            TEST_B.userEventGroupFlag = {};

            a_this->controlMap[RE::InputContextID::kGameplay]->deviceMappings[RE::INPUT_DEVICE::kKeyboard].push_back(TEST_B);
            TEST_B.inputKey = -1;
            a_this->controlMap[RE::InputContextID::kGameplay]->deviceMappings[RE::INPUT_DEVICE::kMouse].push_back(TEST_B);

            CustomEvent TEST_H{ "TEST_H", "TestDocument", "TESTAREA", 2, true };

            TEST_H.inputKey = 35;
            TEST_H.userEventGroupFlag = {};
            a_this->controlMap[RE::InputContextID::kGameplay]->deviceMappings[RE::INPUT_DEVICE::kKeyboard].push_back(TEST_H);
            TEST_H.inputKey = -1;
            a_this->controlMap[RE::InputContextID::kGameplay]->deviceMappings[RE::INPUT_DEVICE::kMouse].push_back(TEST_H);


        }

        inline static REL::Relocation<decltype(thunk)> func;
    };


    //write_branch
    struct [[deprecated("Constructor has no set location and no place to hook.")]] UserEventMappingCtorHook
    {
        
        static void Install()
        {
            //SE: 0xC14030, AE: 0x000000, VR: ???
            auto hook_addr = REL::RelocationID(67275, 00000).address();
           
            //Doing a prologue hook would genuinely be more of a headache than just doing it like this.
            // Should someone also want to hook this (for whatever reason) i'll try to make it play nice, but for now not worth.
            /*
            auto ret_addr = hook_addr + 0x5;
            struct Code : Xbyak::CodeGenerator
            {
                Code(uintptr_t ret_addr)
                {
                    mov(ptr[rsp - 0x18], rcx);
                    mov(rax, ret_addr);
                    jmp(rax);

                }
            } static code{ ret_addr };
            //*/
            auto& trampoline = SKSE::GetTrampoline();

            auto placed_call = IsCallOrJump(hook_addr) > 0;
            
            auto place_query = trampoline.write_branch<5>(hook_addr, (uintptr_t)CustomEvent::CtorImpl2);

            //if (!placed_call)
            //    func = (uintptr_t)code.getCode();
            //else
            //    func = place_query;


            logger::info("UserEventMappingCtorHook complete...");
        }

        static RE::ControlMap::UserEventMapping* thunk(CustomEvent* a_this)
        {
           
            logger::info("eventID: {}, inputKey: {:X}, modifier: {:X}, indexInContext: {}, remappable: {}, linked: {}, userEventGroup: {}",
                a_this->eventID.c_str(),
                a_this->inputKey,
                a_this->modifier,
                a_this->indexInContext,
                a_this->remappable,
                a_this->linked,
                magic_enum::enum_name(a_this->userEventGroupFlag.get()));
            //a_this->pad14 = 0;
            //Just make the argument this idiot.
            a_this->Ctor();
            return a_this;
        }

        inline static REL::Relocation<decltype(thunk)> func;
    };

    
    //asm
    struct UserEventSaveHook
    {

        static void Install()
        {
            //This hook is basically a partial rewrite. No compatibility able to be provided.

            //C11260+ EF - 137
            //SE: 0xC11260, AE: 0x000000, VR: ???
            auto hook_addr = REL::RelocationID(67239, 00000).address();
            auto ret_addr = hook_addr + 0x137;
            struct Code : Xbyak::CodeGenerator
            {
                Code(uintptr_t ret_addr)
                {
                    //Here's the plan, we load the initial number as a parameter into the function, then what value we return we prep to 
                    // put back into the rdi register and go. If we do nothing, we just send it back wholesale.

                    mov(rcx, rbx);
                    mov(rdx, r14);
                    mov(r8w, di);
                    mov(rax, (uintptr_t)thunk);
                    call(rax);

                    mov(di, ax);
                    
                    mov(rax, ret_addr);
                    jmp(rax);

                }
            } static code{ ret_addr };

            auto& trampoline = SKSE::GetTrampoline();

            trampoline.write_branch<5>(hook_addr + 0xEF, (uintptr_t)code.getCode());



            logger::info("UserEventSaveHook complete...");
        }

        static uint16_t thunk(CustomEvent* mapping, uint8_t* buffer, uint16_t index)
        {
            if (mapping->IsCustomEvent() == false) {
                buffer[index++] = mapping->indexInContext;
                buffer[index++] = HIBYTE(mapping->modifier);
                buffer[index++] = LOBYTE(mapping->modifier);
                buffer[index++] = HIBYTE(mapping->inputKey);
                buffer[index++] = LOBYTE(mapping->inputKey);

            }

            return index;

        }

    };


    struct UserEventCategory_TempMappingHook
    {

        static void Install()
        {

            //SE: 0xC13230, AE: 0x000000, VR: ???
            REL::RelocationID base{ 67261, 0 };

            REL::Relocation<uintptr_t> hook{ base, 0x86 };
            
            struct Code : Xbyak::CodeGenerator
            {
                Code()
                {
                    //I'm really not sure where the plus 8 came from but if it works fuck it we ball.
                    // Correction, the +8 comes from the call that just happened to get here. Not our fault, it was already a call.
                    lea(r8, ptr[rsp + 0x88 + -0x50 + 0x8]);
                    //Though, would r13 not suffice? It seems to be loaded manually.

                    mov(rax, (uintptr_t)thunk);
                    jmp(rax);

                }
            } static code{};

            auto& trampoline = SKSE::GetTrampoline();

            func = trampoline.write_call<5>(hook.address(), (uintptr_t)code.getCode());

            //C13230
            //This disables a set of the BSFixedString, preventing our change here from being overriden.
            //REL::safe_fill(base.address() + 0xBA, 0x90, 0x9);
            //REL::safe_fill(base.address() + 0xC3, 0x90, 0x7);

        }

        static bool thunk(CustomEvent& lhs, RE::BSFixedString& rhs, CustomEvent& out)
        {
            auto result = func(lhs, rhs, out);

            

            if (result) {
                //Garbage is in out, we don't want ANY copy constructors firing off, so we have to clear first.
                //reinterpret_cast<void*&>(out.eventID) = nullptr;;
                //out.eventID = lhs.eventID;
                //out.indexInContext = -1;

                //The idea is we use this tag in the temporary space instead of using the fixed string, which may have more complications.
                out.tag() = lhs.mapping()->category();
            }


            return result;

        }


        inline static REL::Relocation<decltype(thunk)> func;
    };

    //write_call
    struct UserEventCategory_CompareHook
    {

        static void Install()
        {
            //C13230+111
            REL::Relocation<uintptr_t> hook{ REL::RelocationID{67261, 0}, 0x111 };

            func = SKSE::GetTrampoline().write_call<5>(hook.address(), thunk);

            logger::info("UserEventCategory_CompareHook complete...");
        }

        static bool thunk(CustomEvent* lhs, CustomEvent* rhs)
        {
           
            //auto category =  



            //auto cmp = lhs->mapping()->category() <=> rhs->mapping()->category();
            auto cmp = lhs->tag() <=> rhs->mapping()->category();

            if (cmp == std::strong_ordering::equivalent)
                return func(lhs, rhs);
            //return func(lhs, rhs);
            return 1;

        }


        inline static REL::Relocation<decltype(thunk)> func;
    };

    struct UserEventCategory_IteratorHook
    {
        static void Install()
        {
            //Safe write to make this  "mov rdx, edx"
           //C13230 + 101
            //ACTUALLY
            //C13230 + F8-101
            //ESI moved into EBX. Hook location might change, for now, I just want the effect, I'll make the proper part later.

            //C13230+111


            auto base = REL::RelocationID(67261, 0).address();
            
            auto hook_address = base + 0xF8;
            auto ret_address = base + 0x101;
            
            //mov ebx, esi
            constexpr std::array<uint8_t, 2> instruction{ 0x89, 0xF3 };
            constexpr auto size = instruction.size();

            REL::safe_write(hook_address, &instruction, size);
            REL::safe_fill(hook_address + size, 0x90, ret_address - hook_address - size);
            logger::info("UserEventCategory_IteratorHook complete...");
        }
    };


}