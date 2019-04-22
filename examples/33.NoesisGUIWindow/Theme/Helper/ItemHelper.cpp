#include "ItemHelper.h"

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
NS_REGISTER_REFLECTION(Material, ItemHelper)
{
	NS_REGISTER_COMPONENT(ItemHelper);
}

Brush * ItemHelper::GetActiveSelectionBackgroundBrush(UIElement * element)
{
	return (element->GetValue<Noesis::Ptr<Brush>>(ActiveSelectionBackgroundBrushProperty));
}

void ItemHelper::SetActiveSelectionBackgroundBrush(UIElement* element, Brush* value)
{
	element->SetValue<Noesis::Ptr<Brush>>(ActiveSelectionBackgroundBrushProperty, value);
}

Brush * ItemHelper::GetActiveSelectionForegroundBrush(UIElement * element)
{
	return (element->GetValue<Noesis::Ptr<Brush>>(ActiveSelectionForegroundBrushProperty));
}

void ItemHelper::SetActiveSelectionForegroundBrush(UIElement* element, Brush* value)
{
	element->SetValue<Noesis::Ptr<Brush>>(ActiveSelectionForegroundBrushProperty, value);
}

Brush * ItemHelper::GetSelectedBackgroundBrush(UIElement * element)
{
	return (element->GetValue<Noesis::Ptr<Brush>>(SelectedBackgroundBrushProperty));
}

void ItemHelper::SetSelectedBackgroundBrush(UIElement* element, Brush* value)
{
	element->SetValue<Noesis::Ptr<Brush>>(SelectedBackgroundBrushProperty, value);
}

Brush * ItemHelper::GetSelectedForegroundBrush(UIElement * element)
{
	return (element->GetValue<Noesis::Ptr<Brush>>(SelectedForegroundBrushProperty));
}

void ItemHelper::SetSelectedForegroundBrush(UIElement* element, Brush* value)
{
	element->SetValue<Noesis::Ptr<Brush>>(SelectedForegroundBrushProperty, value);
}

Brush * ItemHelper::GetHoverBackgroundBrush(UIElement * element)
{
	return (element->GetValue<Noesis::Ptr<Brush>>(HoverBackgroundBrushProperty));
}

void ItemHelper::SetHoverBackgroundBrush(UIElement* element, Brush* value)
{
	element->SetValue<Noesis::Ptr<Brush>>(HoverBackgroundBrushProperty, value);
}

Brush * ItemHelper::GetHoverSelectedBackgroundBrush(UIElement * element)
{
	return (element->GetValue<Noesis::Ptr<Brush>>(HoverSelectedBackgroundBrushProperty));
}

void ItemHelper::SetHoverSelectedBackgroundBrush(UIElement* element, Brush* value)
{
	element->SetValue<Noesis::Ptr<Brush>>(HoverSelectedBackgroundBrushProperty, value);
}

Brush * ItemHelper::GetDisabledSelectedBackgroundBrush(UIElement * element)
{
	return (element->GetValue<Noesis::Ptr<Brush>>(DisabledSelectedBackgroundBrushProperty));
}

void ItemHelper::SetDisabledSelectedBackgroundBrush(UIElement* element, Brush* value)
{
	element->SetValue<Noesis::Ptr<Brush>>(DisabledSelectedBackgroundBrushProperty, value);
}

Brush * ItemHelper::GetDisabledSelectedForegroundBrush(UIElement * element)
{
	return (element->GetValue<Noesis::Ptr<Brush>>(DisabledSelectedForegroundBrushProperty));
}

void ItemHelper::SetDisabledSelectedForegroundBrush(UIElement* element, Brush* value)
{
	element->SetValue<Noesis::Ptr<Brush>>(DisabledSelectedForegroundBrushProperty, value);
}

Brush * ItemHelper::GetDisabledBackgroundBrush(UIElement * element)
{
	return (element->GetValue<Noesis::Ptr<Brush>>(DisabledBackgroundBrushProperty));
}

void ItemHelper::SetDisabledBackgroundBrush(UIElement* element, Brush* value)
{
	element->SetValue<Noesis::Ptr<Brush>>(DisabledBackgroundBrushProperty, value);
}

Brush * ItemHelper::GetDisabledForegroundBrush(UIElement * element)
{
	return (element->GetValue<Noesis::Ptr<Brush>>(DisabledForegroundBrushProperty));
}

void ItemHelper::SetDisabledForegroundBrush(UIElement* element, Brush* value)
{
	element->SetValue<Noesis::Ptr<Brush>>(DisabledForegroundBrushProperty, value);
}

const Noesis::DependencyProperty* ItemHelper::ActiveSelectionBackgroundBrushProperty;
const Noesis::DependencyProperty* ItemHelper::ActiveSelectionForegroundBrushProperty;
const Noesis::DependencyProperty* ItemHelper::SelectedBackgroundBrushProperty;
const Noesis::DependencyProperty* ItemHelper::SelectedForegroundBrushProperty;
const Noesis::DependencyProperty* ItemHelper::HoverBackgroundBrushProperty;
const Noesis::DependencyProperty* ItemHelper::HoverSelectedBackgroundBrushProperty;
const Noesis::DependencyProperty* ItemHelper::DisabledSelectedBackgroundBrushProperty;
const Noesis::DependencyProperty* ItemHelper::DisabledSelectedForegroundBrushProperty;
const Noesis::DependencyProperty* ItemHelper::DisabledBackgroundBrushProperty;
const Noesis::DependencyProperty* ItemHelper::DisabledForegroundBrushProperty;

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(ItemHelper)
{
	NsMeta<TypeId>("WorldClient.Controls.ItemHelper");

	UIElementData* data = NsMeta<UIElementData>(TypeOf<SelfClass>());
	data->RegisterProperty<Noesis::Ptr<Noesis::Brush>>(ActiveSelectionBackgroundBrushProperty, "ActiveSelectionBackgroundBrush",
		PropertyMetadata::Create(Noesis::Ptr<Noesis::Brush>()));
	data->RegisterProperty<Noesis::Ptr<Noesis::Brush>>(ActiveSelectionForegroundBrushProperty, "ActiveSelectionForegroundBrush",
		PropertyMetadata::Create(Noesis::Ptr<Noesis::Brush>()));
	data->RegisterProperty<Noesis::Ptr<Noesis::Brush>>(SelectedBackgroundBrushProperty, "SelectedBackgroundBrush",
		PropertyMetadata::Create(Noesis::Ptr<Noesis::Brush>()));
	data->RegisterProperty<Noesis::Ptr<Noesis::Brush>>(SelectedForegroundBrushProperty, "SelectedForegroundBrush",
		PropertyMetadata::Create(Noesis::Ptr<Noesis::Brush>()));
	data->RegisterProperty<Noesis::Ptr<Noesis::Brush>>(HoverBackgroundBrushProperty, "HoverBackgroundBrush",
		PropertyMetadata::Create(Noesis::Ptr<Noesis::Brush>()));
	data->RegisterProperty<Noesis::Ptr<Noesis::Brush>>(HoverSelectedBackgroundBrushProperty, "HoverSelectedBackgroundBrush",
		PropertyMetadata::Create(Noesis::Ptr<Noesis::Brush>()));
	data->RegisterProperty<Noesis::Ptr<Noesis::Brush>>(DisabledSelectedBackgroundBrushProperty, "DisabledSelectedBackgroundBrush",
		PropertyMetadata::Create(Noesis::Ptr<Noesis::Brush>()));
	data->RegisterProperty<Noesis::Ptr<Noesis::Brush>>(DisabledSelectedForegroundBrushProperty, "DisabledSelectedForegroundBrush",
		PropertyMetadata::Create(Noesis::Ptr<Noesis::Brush>()));
	data->RegisterProperty<Noesis::Ptr<Noesis::Brush>>(DisabledBackgroundBrushProperty, "DisabledBackgroundBrush",
		PropertyMetadata::Create(Noesis::Ptr<Noesis::Brush>()));
	data->RegisterProperty<Noesis::Ptr<Noesis::Brush>>(DisabledForegroundBrushProperty, "DisabledForegroundBrush",
		PropertyMetadata::Create(Noesis::Ptr<Noesis::Brush>()));
}