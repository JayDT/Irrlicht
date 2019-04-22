#include "StringToVisibilityConverter.h"

#include <NsCore/ReflectionImplement.h>
#include <NsCore/TypeId.h>
#include <NsCore/Package.h>
#include <NsApp/IrrNsGuiBindings.h>

using namespace WorldClient::Converters;
using namespace Noesis;

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_REGISTER_REFLECTION(Material, StringToVisibilityConverter)
{
	NS_REGISTER_COMPONENT(StringToVisibilityConverter);
}

StringToVisibilityConverter::StringToVisibilityConverter()
{
	SetFalseEquivalent(Visibility::Visibility_Collapsed);
	SetOppositeStringValue(false);
}

Visibility StringToVisibilityConverter::GetFalseEquivalent() const
{
	return m_FalseEquivalent;
}

void StringToVisibilityConverter::SetFalseEquivalent(Visibility value)
{
	m_FalseEquivalent = value;
}

bool StringToVisibilityConverter::GetOppositeStringValue() const
{
	return m_OppositeStringValue;
}

void StringToVisibilityConverter::SetOppositeStringValue(bool value)
{
	m_OppositeStringValue = value;
}

bool WorldClient::Converters::StringToVisibilityConverter::TryConvert(Noesis::BaseComponent* value, const Noesis::Type* targetType, Noesis::BaseComponent* parameter, Noesis::Ptr<Noesis::BaseComponent>& result)
{
	if (Noesis::Boxing::CanUnbox<eastl::string>(value))
	{
		auto val = Noesis::Boxing::Unbox<eastl::string>(value);
		if (GetOppositeStringValue())
		{
			result = Noesis::Boxing::Box<bool>(val.empty() ? Visibility::Visibility_Visible : GetFalseEquivalent());
		}
		else
		{
			result = Noesis::Boxing::Box<bool>(val.empty() ? GetFalseEquivalent() : Visibility::Visibility_Visible);
		}
		return true;
	}
	return false;
}

bool WorldClient::Converters::StringToVisibilityConverter::TryConvertBack(Noesis::BaseComponent* value, const Noesis::Type* targetType, Noesis::BaseComponent* parameter, Noesis::Ptr<Noesis::BaseComponent>& result)
{
	if (Noesis::Boxing::CanUnbox<Visibility>(value))
	{
		auto val = Noesis::Boxing::Unbox<Visibility>(value);
		if (GetOppositeStringValue())
		{
			result = Noesis::Boxing::Box<eastl::string>(val == Visibility::Visibility_Visible ? "" : "visiable");
		}
		else
		{
			result = Noesis::Boxing::Box<eastl::string>(val == Visibility::Visibility_Visible ? "visiable" : "");
		}
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(StringToVisibilityConverter)
{
	NsMeta<Noesis::TypeId>("WorldClient.Converter.StringToVisibilityConverter");
}