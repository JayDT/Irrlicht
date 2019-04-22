#ifndef __WC_CONTROLSHELPER_H__
#define __WC_CONTROLSHELPER_H__

#include <NsCore/Noesis.h>
#include <NsCore/ReflectionDeclare.h>
#include <NsGui/DependencyObject.h>

namespace WorldClient::Controls
{
    /// <summary>
    /// A helper class that provides various controls.
    /// </summary>
    class ControlsHelper final : public Noesis::DependencyObject
    {
    public:
        static const Noesis::DependencyProperty * DisabledVisualElementVisibilityProperty;
        static const Noesis::DependencyProperty * ContentCharacterCasingProperty;
        static const Noesis::DependencyProperty * HeaderFontFamilyProperty;
        static const Noesis::DependencyProperty * HeaderFontSizeProperty;
        static const Noesis::DependencyProperty * HeaderFontStretchProperty;
		static const Noesis::DependencyProperty * HeaderFontWeightProperty;
        static const Noesis::DependencyProperty * HeaderMarginProperty;
        static const Noesis::DependencyProperty * FocusBorderBrushProperty;
        static const Noesis::DependencyProperty * MouseOverBorderBrushProperty;
        static const Noesis::DependencyProperty * CornerRadiusProperty;
        static const Noesis::DependencyProperty * IsReadOnlyProperty;

        /// <summary>
        /// Gets the value to handle the visibility of the DisabledVisualElement in the template.
        /// </summary>
        static Noesis::Visibility GetDisabledVisualElementVisibility(Noesis::UIElement *element);

        /// <summary>
        /// Sets the value to handle the visibility of the DisabledVisualElement in the template.
        /// </summary>
        static void SetDisabledVisualElementVisibility(Noesis::UIElement *element, Noesis::Visibility value);

        /// <summary>
        /// The DependencyProperty for the CharacterCasing property.
        /// Controls whether or not content is converted to upper or lower case
        /// </summary>

        /// <summary>
        /// Gets the character casing of the control
        /// </summary>
        static Noesis::CharacterCasing GetContentCharacterCasing(Noesis::UIElement *element);

        /// <summary>
        /// Sets the character casing of the control
        /// </summary>
        static void SetContentCharacterCasing(Noesis::UIElement *element, Noesis::CharacterCasing value);

        static Noesis::FontFamily* GetHeaderFontFamily(Noesis::UIElement *element);
        static void SetHeaderFontFamily(Noesis::UIElement *element, Noesis::FontFamily* value);

		static double GetHeaderFontSize(Noesis::UIElement *element);
        static void SetHeaderFontSize(Noesis::UIElement *element, double value);

        static Noesis::FontStretch GetHeaderFontStretch(Noesis::UIElement *element);
        static void SetHeaderFontStretch(Noesis::UIElement *element, Noesis::FontStretch value);

        static Noesis::FontWeight GetHeaderFontWeight(Noesis::UIElement *element);
        static void SetHeaderFontWeight(Noesis::UIElement *element, Noesis::FontWeight value);

		static Noesis::Thickness GetHeaderMargin(Noesis::UIElement *element);
        static void SetHeaderMargin(Noesis::UIElement *element, Noesis::Thickness value);


        /// <summary>
        /// Sets the brush used to draw the focus border.
        /// </summary>
        static void SetFocusBorderBrush(Noesis::DependencyObject *obj, Noesis::Brush *value);

        /// <summary>
        /// Gets the brush used to draw the focus border.
        /// </summary>
        static Noesis::Brush *GetFocusBorderBrush(Noesis::DependencyObject *obj);

        /// <summary>
        /// Sets the brush used to draw the mouse over brush.
        /// </summary>
        static void SetMouseOverBorderBrush(Noesis::DependencyObject *obj, Noesis::Brush *value);

        /// <summary>
        /// Gets the brush used to draw the mouse over brush.
        /// </summary>
        static Noesis::Brush *GetMouseOverBorderBrush(Noesis::DependencyObject *obj);

        /// <summary>
        /// DependencyProperty for <see cref="CornerRadius" /> property.
        /// </summary>

        /// <summary> 
        /// The CornerRadius property allows users to control the roundness of the button corners independently by 
        /// setting a radius value for each corner. Radius values that are too large are scaled so that they
        /// smoothly blend from corner to corner. (Can be used e.g. at MetroButton style)
        /// Description taken from original Microsoft description :-D
        /// </summary>
        static Noesis::CornerRadius GetCornerRadius(Noesis::UIElement *element);

        static void SetCornerRadius(Noesis::UIElement *element, Noesis::CornerRadius value);

        /// <summary>
        /// Gets or sets a value indicating whether the child contents of the control are not editable.
        /// </summary>

        /// <summary>
        /// Gets a value indicating whether the child contents of the control are not editable.
        /// </summary>
        static bool GetIsReadOnly(Noesis::UIElement *element);

        /// <summary>
        /// Sets a value indicating whether the child contents of the control are not editable.
        /// </summary>
        static void SetIsReadOnly(Noesis::UIElement *element, bool value);

	private:
		NS_DECLARE_REFLECTION(ControlsHelper, DependencyObject)
    };
}


#endif	//#ifndef __WC_CONTROLSHELPER_H__
