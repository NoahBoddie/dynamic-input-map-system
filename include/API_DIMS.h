#pragma once

namespace DIMS
{
	struct ConditionData
	{
		//Size helps assertain what members can be accessed.
		const size_t size = sizeof(ConditionData);
		RE::IMenu* openMenu{};

	};

	using ConditionFunction = double(RE::PlayerCharacter*, ConditionData&);



	namespace API
	{
		enum Version
		{
			Version1,

			Current = Version1
		};

		struct InterfaceVersion1
		{
			inline static constexpr auto VERSION = Version::Version1;

			virtual ~InterfaceVersion1() = default;

			/// <summary>
			/// Gets the current version of the interface.
			/// </summary>
			/// <returns></returns>
			[[nodiscard]] virtual Version GetVersion() = 0;



		};

		using CurrentInterface = InterfaceVersion1;


		inline CurrentInterface* Interface{ nullptr };

#ifdef IS_DIMS

		CurrentInterface* InferfaceSingleton();

		namespace detail
		{
			extern "C" __declspec(dllexport) void* DIMS_RequestInterfaceImpl(Version version)
			{
				CurrentInterface* result = InferfaceSingleton();

				switch (version)
				{
				case Version::Version1:
					return dynamic_cast<InterfaceVersion1*>(result);

				}

				return nullptr;
			}
		}

#endif

		/// <summary>
		/// Accesses the ActorValueGenerator Interface. Using the template version is advised. Safe to call PostLoad
		/// </summary>
		/// <param name="version"> to request.</param>
		/// <returns>Returns void* of the interface, cast to the respective version.</returns>
		inline void* RequestInterface(Version version)
		{
			typedef void* (__stdcall* RequestFunction)(Version);

			static RequestFunction request_interface = nullptr;

			HINSTANCE API = GetModuleHandle(L"DynamicInputMapSystem.dll");

			if (API == nullptr) {
				SKSE::log::critical("DynamicInputMapSystem.dll not found, API will remain non functional.");
				return nullptr;
			}

			request_interface = (RequestFunction)GetProcAddress(API, "DIMS_RequestInterfaceImpl");


			if (request_interface) {
				if (static unsigned int once = 0; once++)
					SKSE::log::debug("Successful module and request, DIMS");
			}
			else {
				SKSE::log::warn("Unsuccessful module and request, DIMS");
				return nullptr;
			}

			auto intfc = (CurrentInterface*)request_interface(version);

			return intfc;
		}

		/// <summary>
		/// Accesses the ActorValueGenerator Interface, safe to call PostLoad
		/// </summary>
		/// <typeparam name="InterfaceClass">is the class derived from the interface to use.</typeparam>
		/// <returns>Casts to and returns a specific version of the interface.</returns>
		template <class InterfaceClass = CurrentInterface>
		inline  InterfaceClass* RequestInterface()
		{
			static InterfaceClass* intfc = nullptr;

			if (!intfc) {
				intfc = reinterpret_cast<InterfaceClass*>(RequestInterface(InterfaceClass::VERSION));

				if constexpr (std::is_same_v<InterfaceClass, CurrentInterface>)
					Interface = intfc;
			}

			return intfc;
		}
	}


}
