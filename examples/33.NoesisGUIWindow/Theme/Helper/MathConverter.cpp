#include "MathConverter.h"

#include <NsCore/ReflectionImplement.h>
#include <NsCore/TypeId.h>
#include <NsCore/Package.h>
#include <NsApp/IrrNsGuiBindings.h>

using namespace WorldClient::Converters;

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_REGISTER_REFLECTION(Material, MathConverter)
{
	NS_REGISTER_COMPONENT(MathAddConverter);
	NS_REGISTER_COMPONENT(MathSubtractConverter);
	NS_REGISTER_COMPONENT(MathMultiplyConverter);
	NS_REGISTER_COMPONENT(MathDivideConverter);
}

bool WorldClient::Converters::MathConverter::TryConvert(Noesis::BaseComponent* value, const Noesis::Type* targetType, Noesis::BaseComponent* parameter, Noesis::Ptr<Noesis::BaseComponent>& result)
{
	if (Noesis::Boxing::CanUnbox<float>(value))
	{
		auto val = Noesis::Boxing::Unbox<float>(value);
		auto par = Noesis::Boxing::Unbox<float>(parameter);
		result = Noesis::Boxing::Box<float>(DoConvert(val, par, GetOperation()));
		return true;
	}
	if (Noesis::Boxing::CanUnbox<int>(value))
	{
		auto val = Noesis::Boxing::Unbox<int>(value);
		auto par = Noesis::Boxing::Unbox<int>(parameter);
		result = Noesis::Boxing::Box<int>(DoConvert(val, par, GetOperation()));
		return true;
	}
	if (Noesis::Boxing::CanUnbox<uint32_t>(value))
	{
		auto val = Noesis::Boxing::Unbox<uint32_t>(value);
		auto par = Noesis::Boxing::Unbox<uint32_t>(parameter);
		result = Noesis::Boxing::Box<uint32_t>(DoConvert(val, par, GetOperation()));
		return true;
	}
	return false;
}

bool WorldClient::Converters::MathConverter::TryConvertBack(Noesis::BaseComponent* value, const Noesis::Type* targetType, Noesis::BaseComponent* parameter, Noesis::Ptr<Noesis::BaseComponent>& result)
{
	return false;
}

float MathConverter::DoConvert(float firstValue, float secondValue, MathOperation operation)
{
	auto value1 = firstValue;
	auto value2 = secondValue;

	switch (operation)
	{
		case MathOperation::Add:
			return value1 + value2;
		case MathOperation::Divide:
			return value1 / value2;
		case MathOperation::Multiply:
			return value1 * value2;
		case MathOperation::Subtract:
			return value1 - value2;
		default:
			return value1;
	}

	return value1;
}

MathOperation MathAddConverter::GetOperation() const
{
	return MathOperation::Add;
}

MathOperation MathSubtractConverter::GetOperation() const
{
	return MathOperation::Subtract;
}

MathOperation MathMultiplyConverter::GetOperation() const
{
	return MathOperation::Multiply;
}

MathOperation MathDivideConverter::GetOperation() const
{
	return MathOperation::Divide;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(MathConverter)
{
	NsMeta<Noesis::TypeId>("WorldClient.Converter.MathConverter");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(MathAddConverter)
{
	NsMeta<Noesis::TypeId>("WorldClient.Converter.MathAddConverter");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(MathSubtractConverter)
{
	NsMeta<Noesis::TypeId>("WorldClient.Converter.MathSubtractConverter");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(MathMultiplyConverter)
{
	NsMeta<Noesis::TypeId>("WorldClient.Converter.MathMultiplyConverter");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(MathDivideConverter)
{
	NsMeta<Noesis::TypeId>("WorldClient.Converter.MathDivideConverter");
}