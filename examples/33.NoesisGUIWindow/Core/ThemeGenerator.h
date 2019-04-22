#ifndef __WC_THEME_GENERATOR_H__
#define __WC_THEME_GENERATOR_H__

#include <standard/Errors.h>
#include "rapidjson/rapidjson.h"
#include "rapidjson/writer.h"
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"
#include <map>
#include <vector>
#include <stack>

namespace WorldClient::Core
{
	class XamlThemeDeserializer : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, XamlThemeDeserializer>
	{
	public:
		bool Parse(std::string const& json);
		bool Use(std::string& format_template, std::string const& theme_name);
		bool Use(std::string& format_template, std::map<std::string, std::string> const& theme_values);

		bool Key(const Ch* str, rapidjson::SizeType length, bool copy);
		bool Null();
		bool Bool(bool b);
		bool Int(int32 i);
		bool Uint(uint32 i);
		bool Int64(int64 i);
		bool Uint64(uint64 i);
		bool Double(double d);
		bool String(const Ch* str, rapidjson::SizeType length, bool copy);
		bool StartObject();
		bool EndObject(rapidjson::SizeType memberCount);
		bool StartArray();
		bool EndArray(rapidjson::SizeType memberCount);

		std::vector<std::string> const& GetErrors() const { return mErrors; }

	private:
		struct Arguments
		{
			size_t Start;
			size_t Size;
		};

		struct ThemeObject
		{
			bool IsArray = false;
			std::string Value;

			std::map<std::string, ThemeObject> Values;
			std::list<ThemeObject> Array;

			//const std::string& GetValue(const std::string& name) const
			//{
			//	if (name == Name)
			//		return Value;
			//
			//	auto itr = Values.find(name);
			//	if (itr != Values.end())
			//		return itr->second;
			//	return Value;
			//}
		};

		//bool Apply(const std::string& id, const std::string& value);

		rapidjson::Reader mReader;
		ThemeObject mRootObject;
		ThemeObject* mCurrentValue;
		std::stack<ThemeObject*> mCurrentObject;
		std::multimap<std::string, Arguments> mArguments;
		std::vector<std::string> mErrors;
	};
}

#endif