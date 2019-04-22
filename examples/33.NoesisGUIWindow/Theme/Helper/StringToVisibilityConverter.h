#ifndef __WC_STRINGTOVISIBILITYCONVERTER_H__
#define __WC_STRINGTOVISIBILITYCONVERTER_H__

#include <NsCore/Noesis.h>
#include <NsCore/BaseComponent.h>
#include <NsCore/ReflectionDeclare.h>
#include <NsGui/CoreApi.h>
#include <NsGui/BaseValueConverter.h>

namespace WorldClient::Converters
{
    /// <summary>
    ///     Converts a String into a Visibility enumeration (and back)
    ///     The FalseEquivalent can be declared with the "FalseEquivalent" property
    /// </summary>
    class StringToVisibilityConverter : public Noesis::BaseValueConverter
    {
    private:
		Noesis::Visibility m_FalseEquivalent;
        bool m_OppositeStringValue = false;

    public:
        StringToVisibilityConverter();

        /// <summary>
        ///     FalseEquivalent (default : Visibility.Collapsed => see Constructor)
        /// </summary>
		Noesis::Visibility GetFalseEquivalent() const;
        void SetFalseEquivalent(Noesis::Visibility value);

        /// <summary>
        ///     Define whether the opposite boolean value is crucial (default : false)
        /// </summary>
        bool GetOppositeStringValue() const;
        void SetOppositeStringValue(bool value);

		/// From IValueConverter
		//@{
		bool TryConvert(Noesis::BaseComponent* value, const Noesis::Type* targetType,
			Noesis::BaseComponent* parameter, Noesis::Ptr<Noesis::BaseComponent>& result) override;
		bool TryConvertBack(Noesis::BaseComponent* value, const Noesis::Type* targetType,
			Noesis::BaseComponent* parameter, Noesis::Ptr<Noesis::BaseComponent>& result) override;
		//@}

		NS_IMPLEMENT_INTERFACE_FIXUP

	private:
		NS_DECLARE_REFLECTION(StringToVisibilityConverter, Noesis::BaseValueConverter)
    };
}


#endif	//#ifndef __WC_STRINGTOVISIBILITYCONVERTER_H__
