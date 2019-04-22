////////////////////////////////////////////////////////////////////////////////////////////////////
// NoesisGUI - http://www.noesisengine.com
// Copyright (c) 2013 Noesis Technologies S.L. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////


#include "ElementExtensions.h"

#include <NsCore/ReflectionImplement.h>
#include <NsCore/TypeId.h>
#include <NsGui/FrameworkElement.h>
#include <NsGui/ContentControl.h>
#include <NsGui/UIElementData.h>
#include <NsGui/PropertyMetadata.h>
#include <NsGui/Selector.h>
#include <NsGui/PasswordBox.h>
#include <NsGui/TextBox.h>


using namespace WorldClient;
using namespace Noesis;


////////////////////////////////////////////////////////////////////////////////////////////////////
bool ElementExtensions::GetFocusOnLoaded(DependencyObject* d)
{
    return d->GetValue<bool>(FocusOnLoadedProperty);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ElementExtensions::SetFocusOnLoaded(DependencyObject* d, bool value)
{
    d->SetValue<bool>(FocusOnLoadedProperty, value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool ElementExtensions::GetFocusOnHover(DependencyObject* d)
{
    return d->GetValue<bool>(FocusOnHoverProperty);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ElementExtensions::SetFocusOnHover(DependencyObject* d, bool value)
{
    d->SetValue<bool>(FocusOnHoverProperty, value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool ElementExtensions::GetFocusContentOnHover(DependencyObject* d)
{
    return d->GetValue<bool>(FocusContentOnHoverProperty);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ElementExtensions::SetFocusContentOnHover(DependencyObject* d, bool value)
{
    d->SetValue<bool>(FocusContentOnHoverProperty, value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool ElementExtensions::GetSelectOnHover(DependencyObject* d)
{
    return d->GetValue<bool>(SelectOnHoverProperty);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ElementExtensions::SetSelectOnHover(DependencyObject* d, bool value)
{
    d->SetValue<bool>(SelectOnHoverProperty, value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool ElementExtensions::GetSelectAllOnFocus(DependencyObject* d)
{
    return d->GetValue<bool>(SelectAllOnFocusProperty);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ElementExtensions::SetSelectAllOnFocus(DependencyObject* d, bool value)
{
    d->SetValue<bool>(SelectAllOnFocusProperty, value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ElementExtensions::OnFocusOnLoadedChanged(DependencyObject* d,
        const DependencyPropertyChangedEventArgs& e)
{
    FrameworkElement* element = Noesis::DynamicCast<FrameworkElement*>(d);
    if (element != 0 && *(bool*)e.newValue)
    {
        element->Loaded() += [](BaseComponent* sender, const RoutedEventArgs&)
        {
            FrameworkElement* element = (FrameworkElement*)sender;
            element->Focus();
        };
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ElementExtensions::OnFocusOnHoverChanged(DependencyObject* d,
        const DependencyPropertyChangedEventArgs& e)
{
    FrameworkElement* element = Noesis::DynamicCast<FrameworkElement*>(d);
    if (element != 0 && *(bool*)e.newValue)
    {
        element->Focus();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ElementExtensions::OnFocusContentOnHoverChanged(DependencyObject* d,
    const Noesis::DependencyPropertyChangedEventArgs& e)
{
    ContentControl* control = Noesis::DynamicCast<ContentControl*>(d);
    if (control != 0 && *(bool*)e.newValue)
    {
        UIElement* element = Noesis::DynamicCast<UIElement*>(control->GetContent());
        if (element != 0)
        {
            element->Focus();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ElementExtensions::OnSelectOnHoverChanged(DependencyObject* d,
        const DependencyPropertyChangedEventArgs& e)
{
    FrameworkElement* element = Noesis::DynamicCast<FrameworkElement*>(d);
    if (element != 0 && *(bool*)e.newValue)
    {
        Selector::SetIsSelected(element, true);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ElementExtensions::OnSelectAllOnFocusChanged(DependencyObject* d,
    const DependencyPropertyChangedEventArgs& e)
{
    UIElement* element = Noesis::DynamicCast<UIElement*>(d);
    if (element != 0 && *(bool*)e.newValue)
    {
        element->GotFocus() += [](BaseComponent* sender, const RoutedEventArgs&)
        {
            PasswordBox* passwordBox = Noesis::DynamicCast<PasswordBox*>(sender);
            if (passwordBox != 0)
            {
                passwordBox->SelectAll();
                return;
            }

            TextBox* textBox = Noesis::DynamicCast<TextBox*>(sender);
            if (textBox != 0)
            {
                textBox->SelectAll();
                return;
            }
        };
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(WorldClient::ElementExtensions)
{
    NsMeta<TypeId>("WorldClient.ElementExtensions");

    UIElementData* data = NsMeta<UIElementData>(TypeOf<SelfClass>());
    data->RegisterProperty<bool>(FocusOnLoadedProperty, "FocusOnLoaded",
        PropertyMetadata::Create(false, Noesis::PropertyChangedCallback(&ElementExtensions::OnFocusOnLoadedChanged)));
    data->RegisterProperty<bool>(FocusOnHoverProperty, "FocusOnHover",
        PropertyMetadata::Create(false, Noesis::PropertyChangedCallback(&ElementExtensions::OnFocusOnHoverChanged)));
    data->RegisterProperty<bool>(FocusContentOnHoverProperty, "FocusContentOnHover",
        PropertyMetadata::Create(false, Noesis::PropertyChangedCallback(&ElementExtensions::OnFocusContentOnHoverChanged)));
    data->RegisterProperty<bool>(SelectOnHoverProperty, "SelectOnHover",
        PropertyMetadata::Create(false, Noesis::PropertyChangedCallback(&ElementExtensions::OnSelectOnHoverChanged)));
    data->RegisterProperty<bool>(SelectAllOnFocusProperty, "SelectAllOnFocus",
        PropertyMetadata::Create(false, Noesis::PropertyChangedCallback(&ElementExtensions::OnSelectAllOnFocusChanged)));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
const DependencyProperty* ElementExtensions::FocusOnLoadedProperty;
const DependencyProperty* ElementExtensions::FocusOnHoverProperty;
const DependencyProperty* ElementExtensions::FocusContentOnHoverProperty;
const DependencyProperty* ElementExtensions::SelectOnHoverProperty;
const DependencyProperty* ElementExtensions::SelectAllOnFocusProperty;
