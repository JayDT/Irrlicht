#ifndef __WC_ITEMHELPER_H__
#define __WC_ITEMHELPER_H__

#include <NsCore/Noesis.h>
#include <NsCore/ReflectionDeclare.h>
#include <NsGui/DependencyObject.h>

namespace WorldClient::Controls
{
    class ItemHelper final : public Noesis::DependencyObject
    {
    public:
        static const Noesis::DependencyProperty * ActiveSelectionBackgroundBrushProperty;
        static const Noesis::DependencyProperty * ActiveSelectionForegroundBrushProperty;
        static const Noesis::DependencyProperty * SelectedBackgroundBrushProperty;
        static const Noesis::DependencyProperty * SelectedForegroundBrushProperty;
        static const Noesis::DependencyProperty * HoverBackgroundBrushProperty;
        static const Noesis::DependencyProperty * HoverSelectedBackgroundBrushProperty;
        static const Noesis::DependencyProperty * DisabledSelectedBackgroundBrushProperty;
        static const Noesis::DependencyProperty * DisabledSelectedForegroundBrushProperty;
        static const Noesis::DependencyProperty * DisabledBackgroundBrushProperty;
        static const Noesis::DependencyProperty * DisabledForegroundBrushProperty;

        /// <summary>
        /// Gets the brush the background brush which will be used for the active selected item (if the keyboard focus is within).
        /// </summary>
        static Noesis::Brush *GetActiveSelectionBackgroundBrush(Noesis::UIElement *element);

        /// <summary>
        /// Sets the brush the background brush which will be used for the active selected item (if the keyboard focus is within).
        /// </summary>
        static void SetActiveSelectionBackgroundBrush(Noesis::UIElement *element, Noesis::Brush *value);

        /// <summary>
        /// Gets the brush the foreground brush which will be used for the active selected item (if the keyboard focus is within).
        /// </summary>
        static Noesis::Brush *GetActiveSelectionForegroundBrush(Noesis::UIElement *element);

        /// <summary>
        /// Sets the brush the foreground brush which will be used for the active selected item (if the keyboard focus is within).
        /// </summary>
        static void SetActiveSelectionForegroundBrush(Noesis::UIElement *element, Noesis::Brush *value);

        /// <summary>
        /// Gets the background brush which will be used for a selected item.
        /// </summary>
        static Noesis::Brush *GetSelectedBackgroundBrush(Noesis::UIElement *element);

        /// <summary>
        /// Sets the background brush which will be used for a selected item.
        /// </summary>
        static void SetSelectedBackgroundBrush(Noesis::UIElement *element, Noesis::Brush *value);

        /// <summary>
        /// Gets the foreground brush which will be used for a selected item.
        /// </summary>
        static Noesis::Brush *GetSelectedForegroundBrush(Noesis::UIElement *element);

        /// <summary>
        /// Sets the foreground brush which will be used for a selected item.
        /// </summary>
        static void SetSelectedForegroundBrush(Noesis::UIElement *element, Noesis::Brush *value);

        /// <summary>
        /// Gets the background brush which will be used for an mouse hovered item.
        /// </summary>
        static Noesis::Brush *GetHoverBackgroundBrush(Noesis::UIElement *element);

        /// <summary>
        /// Sets the background brush which will be used for an mouse hovered item.
        /// </summary>
        static void SetHoverBackgroundBrush(Noesis::UIElement *element, Noesis::Brush *value);

        /// <summary>
        /// Gets the background brush which will be used for an mouse hovered and selected item.
        /// </summary>
        static Noesis::Brush *GetHoverSelectedBackgroundBrush(Noesis::UIElement *element);

        /// <summary>
        /// Sets the background brush which will be used for an mouse hovered and selected item.
        /// </summary>
        static void SetHoverSelectedBackgroundBrush(Noesis::UIElement *element, Noesis::Brush *value);

        /// <summary>
        /// Gets the background brush which will be used for selected disabled items.
        /// </summary>
        static Noesis::Brush *GetDisabledSelectedBackgroundBrush(Noesis::UIElement *element);

        /// <summary>
        /// Sets the background brush which will be used for selected disabled items.
        /// </summary>
        static void SetDisabledSelectedBackgroundBrush(Noesis::UIElement *element, Noesis::Brush *value);

        /// <summary>
        /// Gets the foreground brush which will be used for selected disabled items.
        /// </summary>
        static Noesis::Brush *GetDisabledSelectedForegroundBrush(Noesis::UIElement *element);

        /// <summary>
        /// Sets the foreground brush which will be used for selected disabled items.
        /// </summary>
        static void SetDisabledSelectedForegroundBrush(Noesis::UIElement *element, Noesis::Brush *value);

        /// <summary>
        /// Gets the background brush which will be used for disabled items.
        /// </summary>
        static Noesis::Brush *GetDisabledBackgroundBrush(Noesis::UIElement *element);

        /// <summary>
        /// Sets the background brush which will be used for disabled items.
        /// </summary>
        static void SetDisabledBackgroundBrush(Noesis::UIElement *element, Noesis::Brush *value);

        /// <summary>
        /// Gets the foreground brush which will be used for disabled items.
        /// </summary>
        static Noesis::Brush *GetDisabledForegroundBrush(Noesis::UIElement *element);

        /// <summary>
        /// Sets the foreground brush which will be used for disabled items.
        /// </summary>
        static void SetDisabledForegroundBrush(Noesis::UIElement *element, Noesis::Brush *value);

	private:
		NS_DECLARE_REFLECTION(ItemHelper, DependencyObject)
    };
}


#endif	//#ifndef __WC_ITEMHELPER_H__
