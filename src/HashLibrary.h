#pragma once

namespace DIMS
{
	

	struct HashLibrary
	{
		//The purpose of this library is to try to hash everything for the first time, essentially having a link
		// to when a thing is made. Note, this need not be used all the time, just when some

		inline static std::unordered_map<uint32_t, std::string> smallHashRegister;
		inline static std::unordered_map<size_t, std::string> largeHashRegister;



	};


	//The idea behind the both of these is I actually don't really need to access the string most of the time,
	// but when I do this should make it a little easier.
	struct LittleString
	{
		uint32_t value;
	};

	struct BigString
	{

	};
}