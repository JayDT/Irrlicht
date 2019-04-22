#include "IsNullConverter.h"
#include <NsCore/ReflectionImplement.h>
#include <NsCore/TypeId.h>
#include <NsCore/Package.h>
#include <NsApp/IrrNsGuiBindings.h>

using namespace WorldClient::Converters;

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_REGISTER_REFLECTION(Material, IsNullConverter)
{
	NS_REGISTER_COMPONENT(IsNullConverter);
}

bool WorldClient::Converters::IsNullConverter::TryConvert(Noesis::BaseComponent* value, const Noesis::Type* targetType, Noesis::BaseComponent* parameter, Noesis::Ptr<Noesis::BaseComponent>& result)
{
	result = Noesis::Boxing::Box<bool>(value != nullptr);
	return true;
}

bool WorldClient::Converters::IsNullConverter::TryConvertBack(Noesis::BaseComponent* value, const Noesis::Type* targetType, Noesis::BaseComponent* parameter, Noesis::Ptr<Noesis::BaseComponent>& result)
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_BEGIN_COLD_REGION

NS_IMPLEMENT_REFLECTION(IsNullConverter)
{
	NsMeta<Noesis::TypeId>("WorldClient.Converters.IsNullConverter");
}