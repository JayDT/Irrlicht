#include "StdAfx.h"
#include "ThemeGenerator.h"

using namespace WorldClient::Core;

bool XamlThemeDeserializer::Parse(std::string const& json)
{
	mCurrentValue = &mRootObject;
	rapidjson::StringStream ss(json.c_str());
	rapidjson::ParseResult result = mReader.Parse(ss, *this);
	//ASSERT(result.IsError() || (_objectState.empty() && _state.empty()));
	return !result.IsError() && mErrors.empty();
}

bool WorldClient::Core::XamlThemeDeserializer::Use(std::string& format_template, std::string const& theme_name)
{
	auto names = System::String(theme_name).Split('.');
	ThemeObject* selected = &mRootObject;
	for (auto name : names)
	{
		if (selected->IsArray)
		{
			bool found = false;
			for (auto& element : selected->Array)
			{
				if (element.Values["Name"].Value == name)
				{
					selected = &element.Values["Values"];
					found = true;
					break;
				}
			}

			if (!found)
				return false;
		}
		else
		{
			auto iSel = selected->Values.find(name);
			if (iSel == selected->Values.end())
				return false;

			selected = &iSel->second;
		}
	}


	for (auto& element : selected->Values)
	{
		size_t pos = 0;
		std::string find = System::String::format("{{%s}}", element.first.c_str());
		size_t findLen = find.size();
		while ((pos = format_template.find(find, pos)) != std::string::npos)
			format_template.replace(pos, findLen, element.second.Value);
	}

	return true;
}

bool WorldClient::Core::XamlThemeDeserializer::Use(std::string& format_template, std::map<std::string, std::string> const& theme_values)
{
	for (auto& element : theme_values)
	{
		size_t pos = 0;
		std::string find = System::String::format("{{%s}}", element.first.c_str());
		size_t findLen = find.size();
		while ((pos = format_template.find(find, pos)) != std::string::npos)
			format_template.replace(pos, findLen, element.second);
	}
	return false;
}

bool XamlThemeDeserializer::Key(const Ch* str, rapidjson::SizeType length, bool copy)
{
	mCurrentValue = &mCurrentObject.top()->Values[str];
	return true;
}

bool XamlThemeDeserializer::Null()
{
	return true;
}

bool XamlThemeDeserializer::Bool(bool b)
{
	return true;
}

bool XamlThemeDeserializer::Int(int32 i)
{
	return true;
}

bool XamlThemeDeserializer::Uint(uint32 i)
{
	return true;
}

bool XamlThemeDeserializer::Int64(int64 i)
{
	return true;
}

bool XamlThemeDeserializer::Uint64(uint64 i)
{
	return true;
}

bool XamlThemeDeserializer::Double(double d)
{
	return true;
}

bool XamlThemeDeserializer::String(const Ch* str, rapidjson::SizeType length, bool copy)
{
	if (mCurrentValue)
		mCurrentValue->Value.append(str, length);
	return true;
}

bool XamlThemeDeserializer::StartObject()
{
	if (!mCurrentObject.empty() && mCurrentObject.top()->IsArray)
	{
		mCurrentObject.push(&mCurrentObject.top()->Array.emplace_back());
	}
	else
	{
		mCurrentObject.push(mCurrentValue);
	}
	return true;
}

bool XamlThemeDeserializer::EndObject(rapidjson::SizeType memberCount)
{
	mCurrentObject.pop();
	if (!mCurrentObject.empty())
		mCurrentValue = mCurrentObject.top();
	return true;
}

bool XamlThemeDeserializer::StartArray()
{
	mCurrentValue->IsArray = true;
	mCurrentObject.push(mCurrentValue);
	return true;
}

bool XamlThemeDeserializer::EndArray(rapidjson::SizeType memberCount)
{
	mCurrentObject.pop();
	mCurrentValue = mCurrentObject.top();
	return true;
}
