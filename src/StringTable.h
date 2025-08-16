#pragma once

namespace DIMS
{
	class StringTable
	{
		inline static std::unordered_map<StringHash, std::string> hashTable;

	public:

		constexpr static StringHash Hash(const std::string_view& str)
		{
			auto hash = RGL::Hash<HashFlags::Insensitive>(str);

			if (!std::is_constant_evaluated())
			{
				if (auto& entry = hashTable[hash]; entry.empty())
					entry = std::string{ str };
			}

			return hash;
		}

		static std::string_view FindString(StringHash hash)
		{
			auto it = hashTable.find(hash);

			if (hashTable.end() != it) {
				return it->second;
			}

			return {};
		}

	};

}