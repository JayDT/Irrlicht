#include "ToggleButtonHelper.h"

#include <NsCore/ReflectionImplement.h>
#include <NsCore/TypeId.h>
#include <NsGui/UIElement.h>
#include <NsGui/ContentControl.h>
#include <NsGui/UIElementData.h>
#include <NsGui/PropertyMetadata.h>
#include <NsGui/Selector.h>
#include <NsGui/PasswordBox.h>
#include <NsGui/TextBox.h>
#include <NsApp/IrrNsGuiBindings.h>

using namespace WorldClient::Controls;
using namespace Noesis;

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_REGISTER_REFLECTION(Material, ToggleButtonHelper)
{
	NS_REGISTER_COMPONENT(ToggleButtonHelper);
}

FlowDirection ToggleButtonHelper::GetContentDirection(UIElement *element)
{
    return (element->GetValue<FlowDirection>(ContentDirectionProperty));
}

void ToggleButtonHelper::SetContentDirection(UIElement *element, FlowDirection value)
{
    element->SetValue<FlowDirection>(ContentDirectionProperty, value);
}

void ToggleButtonHelper::ContentDirectionPropertyChanged(DependencyObject *d, const DependencyPropertyChangedEventArgs& e)
{
    //auto tb = dynamic_cast<ToggleButton*>(d);
    //if (nullptr == tb)
    //{
    //    throw System::InvalidOperationException("The property 'ContentDirection' may only be set on ToggleButton elements.");
    //}
}

const Noesis::DependencyProperty* ToggleButtonHelper::ContentDirectionProperty;

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(ToggleButtonHelper)
{
	NsMeta<TypeId>("WorldClient.Controls.ToggleButtonHelper");

	UIElementData* data = NsMeta<UIElementData>(TypeOf<SelfClass>());
	data->RegisterProperty<FlowDirection>(ContentDirectionProperty, "ContentDirection",
		PropertyMetadata::Create(FlowDirection::FlowDirection_LeftToRight));
}