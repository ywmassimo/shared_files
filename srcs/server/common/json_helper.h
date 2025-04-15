#ifndef INCLUDE_COMMON_JSON_HELPER_H
#define INCLUDE_COMMON_JSON_HELPER_H
#include <optional>
#include <rapidjson/document.h>
namespace jsonHelper
{
	namespace details {
		inline std::optional<int> getInt(rapidjson::Value& value, const std::string& key) {
			// checking member exists
			if (!value.HasMember(key.c_str()))
				return std::nullopt;

			//getting member reference
			rapidjson::Value& member = value[key.c_str()];

			// checking for integer
			if (member.IsInt() == false)
				return std::nullopt;

			// making optional with value
			return std::optional<int>{member.GetInt()};
		}

		inline std::optional<int64_t> getInt64(rapidjson::Value& value, const std::string& key) {
			// checking member exists
			if (!value.HasMember(key.c_str()))
				return std::nullopt;

			//getting member reference
			rapidjson::Value& member = value[key.c_str()];

			// checking for integer
			if (member.IsInt64() == false)
				return std::nullopt;

			// making optional with value
			return std::optional<int64_t>{member.GetInt64()};
		}

		inline std::optional<std::string> getString(rapidjson::Value& value, const std::string& key) {
			// checking member exists
			if (!value.HasMember(key.c_str()))
				return std::nullopt;

			//getting member reference
			rapidjson::Value& member = value[key.c_str()];

			// checking for integer
			if (member.IsString() == false)
				return std::nullopt;

			// making optional with value
			return std::optional<std::string>{member.GetString()};
		}
	}

	template<class T>
	inline std::optional<T> getValue(rapidjson::Value& value, const std::string& key) {
		if constexpr (std::is_same_v<T, int>)
			return details::getInt(value, key);
		else if constexpr (std::is_same_v<T, int64_t>)
			return details::getInt64(value, key);
		else if constexpr (std::is_same_v<T, std::string>)
			return details::getString(value, key);
		else
			return std::nullopt;
	}


	inline void insertValue(rapidjson::Value& value, const std::string& key, const int& insertValue, rapidjson::Document::AllocatorType& allocator) {
		assert(value.IsObject());
		rapidjson::Value keyValue;
		keyValue.SetString(key.c_str(), allocator);
		rapidjson::Value newValue;
		newValue.SetInt(insertValue);
		value.AddMember(keyValue, newValue, allocator);
	}

	inline void insertValue(rapidjson::Value& value, const std::string& key, const int64_t& insertValue, rapidjson::Document::AllocatorType& allocator) {
		assert(value.IsObject());
		rapidjson::Value keyValue;
		keyValue.SetString(key.c_str(), allocator);
		rapidjson::Value newValue;
		newValue.SetInt64(insertValue);
		value.AddMember(keyValue, newValue, allocator);
	}

	inline void insertValue(rapidjson::Value& value, const std::string& key, const std::string& insertValue, rapidjson::Document::AllocatorType& allocator) {
		assert(value.IsObject());
		rapidjson::Value keyValue;
		keyValue.SetString(key.c_str(), allocator);
		rapidjson::Value newValue;
		newValue.SetString(insertValue.c_str(), allocator);
		value.AddMember(keyValue, newValue, allocator);
	}
}

#endif //INCLUDE_COMMON_JSON_HELPER_H
//martysama0134's b9b96f11f29d7eae7e2d2a9beb08ec8f
