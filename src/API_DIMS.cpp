#include "API_DIMS.h"

namespace DIMS::API
{
	struct DIMSInterface : public CurrentInterface
	{
		Version GetVersion() override { return Version::Current; }

	};

	[[nodiscard]] CurrentInterface* InferfaceSingleton()
	{
		static DIMSInterface intfc{};
		return &intfc;
	}
}