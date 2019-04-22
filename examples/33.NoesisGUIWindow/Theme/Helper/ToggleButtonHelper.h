#ifndef __WC_TOGGLEBUTTONHELPER_H__
#define __WC_TOGGLEBUTTONHELPER_H__

#include <NsCore/Noesis.h>
#include <NsCore/ReflectionDeclare.h>
#include <NsGui/DependencyObject.h>

namespace WorldClient::Controls
{

    class ToggleButtonHelper final : public Noesis::BaseComponent
    {
    public:
        /// <summary>
        /// This property can be used to handle the style for CheckBox and RadioButton
        /// LeftToRight means content left and button right and RightToLeft vise versa
        /// </summary>
        static const Noesis::DependencyProperty * ContentDirectionProperty;

        /// <summary>
        /// This property can be used to handle the style for CheckBox and RadioButton
        /// LeftToRight means content left and button right and RightToLeft vise versa
        /// </summary>
        static Noesis::FlowDirection GetContentDirection(Noesis::UIElement *element);
        static void SetContentDirection(Noesis::UIElement *element, Noesis::FlowDirection value);

    private:
        static void ContentDirectionPropertyChanged(Noesis::DependencyObject *d, const Noesis::DependencyPropertyChangedEventArgs& e);

	private:
		NS_DECLARE_REFLECTION(ToggleButtonHelper, Noesis::BaseComponent)
    };
}


#endif	//#ifndef __WC_TOGGLEBUTTONHELPER_H__
