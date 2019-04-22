#ifndef __WC_MATHCONVERTER_H__
#define __WC_MATHCONVERTER_H__

#include <NsCore/Noesis.h>
#include <NsCore/BaseComponent.h>
#include <NsCore/ReflectionDeclare.h>
#include <NsGui/CoreApi.h>
#include <NsGui/BaseValueConverter.h>

namespace WorldClient::Converters
{
    /// <summary>
    /// The math operations which can be used at the <see cref="MathConverter"/>
    /// </summary>
    enum class MathOperation
    {
        Add,
        Subtract,
        Multiply,
        Divide
    };

    /// <summary>
    /// MathConverter provides a value converter which can be used for math operations.
    /// It can be used for normal binding or multi binding as well.
    /// If it is used for normal binding the given parameter will be used as operands with the selected operation.
    /// If it is used for multi binding then the first and second binding will be used as operands with the selected operation.
    /// This class cannot be inherited.
    /// </summary>
    class MathConverter : public Noesis::BaseValueConverter
    {
    public:
		virtual ~MathConverter() {}
        virtual MathOperation GetOperation() const = 0;

		/// From IValueConverter
		//@{
		bool TryConvert(Noesis::BaseComponent* value, const Noesis::Type* targetType,
			Noesis::BaseComponent* parameter, Noesis::Ptr<Noesis::BaseComponent>& result) override;
		bool TryConvertBack(Noesis::BaseComponent* value, const Noesis::Type* targetType,
			Noesis::BaseComponent* parameter, Noesis::Ptr<Noesis::BaseComponent>& result) override;
		//@}

		NS_IMPLEMENT_INTERFACE_FIXUP

		private:
			float DoConvert(float firstValue, float secondValue, MathOperation operation);

		private:
			NS_DECLARE_REFLECTION(MathConverter, Noesis::BaseValueConverter)
    };

    /// <summary>
    /// MathAddConverter provides a multi value converter as a MarkupExtension which can be used for math operations.
    /// This class cannot be inherited.
    /// </summary>
    class MathAddConverter final : public MathConverter
    {
		public:
			MathOperation GetOperation() const override;

		private:
			NS_DECLARE_REFLECTION(MathAddConverter, MathConverter)
	};

    /// <summary>
    /// MathSubtractConverter provides a multi value converter as a MarkupExtension which can be used for math operations.
    /// This class cannot be inherited.
    /// </summary>
    class MathSubtractConverter final : public MathConverter
    {
		public:
			MathOperation GetOperation() const override;

		private:
			NS_DECLARE_REFLECTION(MathSubtractConverter, MathConverter)
    };

    /// <summary>
    /// MathMultiplyConverter provides a multi value converter as a MarkupExtension which can be used for math operations.
    /// This class cannot be inherited.
    /// </summary>
    class MathMultiplyConverter final : public MathConverter
    {
		public:
			MathOperation GetOperation() const override;

		private:
			NS_DECLARE_REFLECTION(MathMultiplyConverter, MathConverter)
    };

    /// <summary>
    /// MathDivideConverter provides a multi value converter as a MarkupExtension which can be used for math operations.
    /// This class cannot be inherited.
    /// </summary>
    class MathDivideConverter final : public MathConverter
    {
		public:
			MathOperation GetOperation() const override;

		private:
			NS_DECLARE_REFLECTION(MathDivideConverter, MathConverter)
    };
}


#endif	//#ifndef __WC_MATHCONVERTER_H__
