#ifndef __WC_ISNULLCONVERTER_H__
#define __WC_ISNULLCONVERTER_H__

#include <NsCore/Noesis.h>
#include <NsCore/BaseComponent.h>
#include <NsCore/ReflectionDeclare.h>
#include <NsGui/CoreApi.h>
#include <NsGui/BaseValueConverter.h>

namespace WorldClient::Converters
{
    /// <summary>
    /// Converts the value from true to false and false to true.
    /// </summary>
    class IsNullConverter final : public Noesis::BaseValueConverter
    {
    public:
        IsNullConverter() {}
		virtual ~IsNullConverter() {}

		/// From IValueConverter
		//@{
		bool TryConvert(Noesis::BaseComponent* value, const Noesis::Type* targetType,
			Noesis::BaseComponent* parameter, Noesis::Ptr<Noesis::BaseComponent>& result) override;
		bool TryConvertBack(Noesis::BaseComponent* value, const Noesis::Type* targetType,
			Noesis::BaseComponent* parameter, Noesis::Ptr<Noesis::BaseComponent>& result) override;
		//@}

		NS_IMPLEMENT_INTERFACE_FIXUP

	private:
		NS_DECLARE_REFLECTION(IsNullConverter, Noesis::BaseValueConverter)
    };
}


#endif	//#ifndef __WC_ISNULLCONVERTER_H__
