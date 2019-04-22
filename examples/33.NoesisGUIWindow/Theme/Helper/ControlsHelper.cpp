#include "ControlsHelper.h"

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
NS_REGISTER_REFLECTION(Material, ControlsHelper)
{
	NS_REGISTER_COMPONENT(ControlsHelper);
}

Visibility ControlsHelper::GetDisabledVisualElementVisibility(UIElement * element)
{
	return (element->GetValue<Visibility>(DisabledVisualElementVisibilityProperty));
}

void ControlsHelper::SetDisabledVisualElementVisibility(UIElement* element, Visibility value)
{
	element->SetValue<Visibility>(DisabledVisualElementVisibilityProperty, value);
}

CharacterCasing ControlsHelper::GetContentCharacterCasing(UIElement* element)
{
	return (element->GetValue<CharacterCasing>(ContentCharacterCasingProperty));
}

void ControlsHelper::SetContentCharacterCasing(UIElement* element, CharacterCasing value)
{
	element->SetValue<CharacterCasing>(ContentCharacterCasingProperty, value);
}

FontFamily* ControlsHelper::GetHeaderFontFamily(UIElement* element)
{
	return (element->GetValue<Noesis::Ptr<FontFamily>>(HeaderFontFamilyProperty).GetPtr());
}

void ControlsHelper::SetHeaderFontFamily(UIElement* element, FontFamily* value)
{
	element->SetValue<Noesis::Ptr<FontFamily>>(HeaderFontFamilyProperty, value);
}

double ControlsHelper::GetHeaderFontSize(UIElement* element)
{
	return (element->GetValue<double>(HeaderFontSizeProperty));
}

void ControlsHelper::SetHeaderFontSize(UIElement* element, double value)
{
	element->SetValue<double>(HeaderFontSizeProperty, value);
}

FontStretch ControlsHelper::GetHeaderFontStretch(UIElement* element)
{
	return (element->GetValue<FontStretch>(HeaderFontStretchProperty));
}

void ControlsHelper::SetHeaderFontStretch(UIElement* element, FontStretch value)
{
	element->SetValue<FontStretch>(HeaderFontStretchProperty, value);
}

FontWeight ControlsHelper::GetHeaderFontWeight(UIElement* element)
{
	return (element->GetValue<FontWeight>(HeaderFontWeightProperty));
}

void ControlsHelper::SetHeaderFontWeight(UIElement* element, FontWeight value)
{
	element->SetValue<FontWeight>(HeaderFontWeightProperty, value);
}

Thickness ControlsHelper::GetHeaderMargin(UIElement* element)
{
	return (element->GetValue<Thickness>(HeaderMarginProperty));
}

void ControlsHelper::SetHeaderMargin(UIElement* element, Thickness value)
{
	element->SetValue<Thickness>(HeaderMarginProperty, value);
}

void ControlsHelper::SetFocusBorderBrush(DependencyObject * obj, Brush * value)
{
	obj->SetValue<Noesis::Ptr<Brush>>(FocusBorderBrushProperty, value);
}

Brush* ControlsHelper::GetFocusBorderBrush(DependencyObject* obj)
{
	return (obj->GetValue<Noesis::Ptr<Brush>>(FocusBorderBrushProperty).GetPtr());
}

void ControlsHelper::SetMouseOverBorderBrush(DependencyObject* obj, Brush* value)
{
	obj->SetValue<Noesis::Ptr<Brush>>(MouseOverBorderBrushProperty, value);
}

Brush* ControlsHelper::GetMouseOverBorderBrush(DependencyObject* obj)
{
	return(obj->GetValue<Noesis::Ptr<Brush>>(MouseOverBorderBrushProperty).GetPtr());
}

CornerRadius ControlsHelper::GetCornerRadius(UIElement * element)
{
	return (element->GetValue<CornerRadius>(CornerRadiusProperty));
}

void ControlsHelper::SetCornerRadius(UIElement* element, CornerRadius value)
{
	element->SetValue<CornerRadius>(CornerRadiusProperty, value);
}

bool ControlsHelper::GetIsReadOnly(UIElement* element)
{
	return (element->GetValue<bool>(IsReadOnlyProperty));
}

void ControlsHelper::SetIsReadOnly(UIElement* element, bool value)
{
	element->SetValue<bool>(IsReadOnlyProperty, value);
}

const Noesis::DependencyProperty* ControlsHelper::DisabledVisualElementVisibilityProperty;
const Noesis::DependencyProperty* ControlsHelper::ContentCharacterCasingProperty;
const Noesis::DependencyProperty* ControlsHelper::HeaderFontFamilyProperty;
const Noesis::DependencyProperty* ControlsHelper::HeaderFontSizeProperty;
const Noesis::DependencyProperty* ControlsHelper::HeaderFontStretchProperty;
const Noesis::DependencyProperty* ControlsHelper::HeaderFontWeightProperty;
const Noesis::DependencyProperty* ControlsHelper::HeaderMarginProperty;
const Noesis::DependencyProperty* ControlsHelper::FocusBorderBrushProperty;
const Noesis::DependencyProperty* ControlsHelper::MouseOverBorderBrushProperty;
const Noesis::DependencyProperty* ControlsHelper::CornerRadiusProperty;
const Noesis::DependencyProperty* ControlsHelper::IsReadOnlyProperty;

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(ControlsHelper)
{
	NsMeta<TypeId>("WorldClient.Controls.ControlsHelper");

	UIElementData* data = NsMeta<UIElementData>(TypeOf<SelfClass>());
	data->RegisterProperty<Noesis::Visibility>(DisabledVisualElementVisibilityProperty, "DisabledVisualElementVisibility",
		PropertyMetadata::Create(Noesis::Visibility::Visibility_Visible));
	data->RegisterProperty<CharacterCasing>(ContentCharacterCasingProperty, "ContentCharacterCasing",
		PropertyMetadata::Create(CharacterCasing::CharacterCasing_Normal));
	data->RegisterProperty<Noesis::Ptr<FontFamily>>(HeaderFontFamilyProperty, "HeaderFontFamily",
		PropertyMetadata::Create(Noesis::Ptr<FontFamily>()));
	data->RegisterProperty<double>(HeaderFontSizeProperty, "HeaderFontSize",
		PropertyMetadata::Create(double(0.0f)));
	data->RegisterProperty<Noesis::FontStretch>(HeaderFontStretchProperty, "HeaderFontStretch",
		PropertyMetadata::Create(Noesis::FontStretch::FontStretch_Normal));
	data->RegisterProperty<FontWeight>(HeaderFontWeightProperty, "HeaderFontWeight",
		PropertyMetadata::Create(FontWeight::FontWeight_Normal));
	data->RegisterProperty<Thickness>(HeaderMarginProperty, "HeaderMargin",
		PropertyMetadata::Create(Thickness(0.0f)));
	data->RegisterProperty<Noesis::Ptr<Noesis::Brush>>(FocusBorderBrushProperty, "FocusBorderBrush",
		PropertyMetadata::Create(Noesis::Ptr<Noesis::Brush>()));
	data->RegisterProperty<Noesis::Ptr<Noesis::Brush>>(MouseOverBorderBrushProperty, "MouseOverBorderBrush",
		PropertyMetadata::Create(Noesis::Ptr<Noesis::Brush>()));
	data->RegisterProperty<CornerRadius>(CornerRadiusProperty, "CornerRadius",
		PropertyMetadata::Create(CornerRadius()));
	data->RegisterProperty<bool>(IsReadOnlyProperty, "IsReadOnly",
		PropertyMetadata::Create(false));
}

