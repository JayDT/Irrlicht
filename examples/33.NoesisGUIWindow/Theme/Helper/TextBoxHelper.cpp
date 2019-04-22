#include "TextBoxHelper.h"

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
NS_REGISTER_REFLECTION(Material, TextBoxHelper)
{
	NsRegisterComponent<EnumConverter<WorldClient::Controls::ButtonsAlignment>>();
	NsRegisterComponent<EnumConverter<WorldClient::Controls::SpellingResourceKeyId>>();

	NS_REGISTER_COMPONENT(TextChangedEventArgs);
	NS_REGISTER_COMPONENT(TextBoxHelper);
}

bool WorldClient::Controls::TextBoxHelper::GetIsSpellCheckContextMenuEnabled(Noesis::UIElement* element)
{
	return element->GetValue<bool>(IsSpellCheckContextMenuEnabledProperty);
}

void WorldClient::Controls::TextBoxHelper::SetIsSpellCheckContextMenuEnabled(Noesis::UIElement* element, bool value)
{
	element->SetValue<bool>(IsSpellCheckContextMenuEnabledProperty, value);
}

bool WorldClient::Controls::TextBoxHelper::GetAutoWatermark(Noesis::DependencyObject* element)
{
	return element->GetValue<bool>(AutoWatermarkProperty);
}

void WorldClient::Controls::TextBoxHelper::SetAutoWatermark(Noesis::DependencyObject* element, bool value)
{
	element->SetValue<bool>(AutoWatermarkProperty, value);
}

void WorldClient::Controls::TextBoxHelper::OnAutoWatermarkChanged(Noesis::DependencyObject* d, Noesis::DependencyPropertyChangedEventArgs& e)
{
}

void WorldClient::Controls::TextBoxHelper::OnControlWithAutoWatermarkSupportLoaded(Noesis::BaseComponent* o, Noesis::RoutedEventArgs& routedEventArgs)
{
}

void WorldClient::Controls::TextBoxHelper::IsSpellCheckContextMenuEnabledChanged(Noesis::DependencyObject* d, Noesis::DependencyPropertyChangedEventArgs& e)
{
}

void WorldClient::Controls::TextBoxHelper::TextBoxBaseLostFocus(Noesis::BaseComponent* sender, Noesis::RoutedEventArgs& e)
{
}

void WorldClient::Controls::TextBoxHelper::TextBoxBaseContextMenuClosing(Noesis::BaseComponent* sender, Noesis::ContextMenuEventArgs& e)
{
}

void WorldClient::Controls::TextBoxHelper::TextBoxBaseContextMenuOpening(Noesis::BaseComponent* sender, Noesis::ContextMenuEventArgs& e)
{
}

void WorldClient::Controls::TextBoxHelper::RemoveSpellCheckMenuItems(Noesis::ContextMenu* contextMenu)
{
}

void WorldClient::Controls::TextBoxHelper::SetIsWaitingForData(Noesis::DependencyObject* obj, bool value)
{
}

bool WorldClient::Controls::TextBoxHelper::GetIsWaitingForData(Noesis::DependencyObject* obj)
{
	return obj->GetValue<bool>(IsWaitingForDataProperty);
}

void WorldClient::Controls::TextBoxHelper::SetSelectAllOnFocus(Noesis::DependencyObject* obj, bool value)
{
	obj->SetValue<bool>(IsWaitingForDataProperty, value);
}

bool WorldClient::Controls::TextBoxHelper::GetSelectAllOnFocus(Noesis::DependencyObject* obj)
{
	return obj->GetValue<bool>(SelectAllOnFocusProperty);
}

void WorldClient::Controls::TextBoxHelper::SetIsMonitoring(Noesis::DependencyObject* obj, bool value)
{
	obj->SetValue<bool>(SelectAllOnFocusProperty, value);
}

const char* WorldClient::Controls::TextBoxHelper::GetWatermark(Noesis::DependencyObject* obj)
{
	return obj->GetValue<eastl::string>(WatermarkProperty).c_str();
}

void WorldClient::Controls::TextBoxHelper::SetWatermark(Noesis::DependencyObject* obj, const char* value)
{
	obj->SetValue<eastl::string>(WatermarkProperty, value);
}

Noesis::TextAlignment WorldClient::Controls::TextBoxHelper::GetWatermarkAlignment(DependencyObject* obj)
{
	return obj->GetValue<Noesis::TextAlignment>(WatermarkAlignmentProperty);
}

void WorldClient::Controls::TextBoxHelper::SetWatermarkAlignment(DependencyObject* obj, Noesis::TextAlignment value)
{
	obj->SetValue<Noesis::TextAlignment>(WatermarkAlignmentProperty, value);
}

Noesis::TextTrimming WorldClient::Controls::TextBoxHelper::GetWatermarkTrimming(DependencyObject* obj)
{
	return obj->GetValue<Noesis::TextTrimming>(WatermarkTrimmingProperty);
}

void WorldClient::Controls::TextBoxHelper::SetWatermarkTrimming(DependencyObject* obj, Noesis::TextTrimming value)
{
	obj->SetValue<Noesis::TextTrimming>(WatermarkTrimmingProperty, value);
}

Noesis::TextWrapping WorldClient::Controls::TextBoxHelper::GetWatermarkWrapping(DependencyObject* obj)
{
	return obj->GetValue<Noesis::TextWrapping>(WatermarkWrappingProperty);
}

void WorldClient::Controls::TextBoxHelper::SetWatermarkWrapping(Noesis::DependencyObject* obj, Noesis::TextWrapping value)
{
	obj->SetValue<Noesis::TextWrapping>(WatermarkWrappingProperty, value);
}

bool WorldClient::Controls::TextBoxHelper::GetUseFloatingWatermark(Noesis::DependencyObject* obj)
{
	return obj->GetValue<bool>(UseFloatingWatermarkProperty);
}

void WorldClient::Controls::TextBoxHelper::SetUseFloatingWatermark(Noesis::DependencyObject* obj, bool value)
{
	obj->SetValue<bool>(UseFloatingWatermarkProperty, value);
}

bool WorldClient::Controls::TextBoxHelper::GetHasText(Noesis::DependencyObject* obj)
{
	return obj->GetValue<bool>(HasTextProperty);
}

void WorldClient::Controls::TextBoxHelper::SetHasText(Noesis::DependencyObject* obj, bool value)
{
	obj->SetValue<bool>(HasTextProperty, value);
}

int32_t WorldClient::Controls::TextBoxHelper::GetTextLength(Noesis::DependencyObject* obj)
{
	return obj->GetValue<int32_t>(TextLengthProperty);
}

void WorldClient::Controls::TextBoxHelper::SetTextLength(Noesis::DependencyObject* obj, int32_t value)
{
	obj->SetValue<int32_t>(TextLengthProperty, value);
}

void WorldClient::Controls::TextBoxHelper::OnIsMonitoringChanged(Noesis::DependencyObject* d, Noesis::DependencyPropertyChangedEventArgs& e)
{
}

void WorldClient::Controls::TextBoxHelper::RichTextBox_TextChanged(Noesis::BaseComponent* sender, TextChangedEventArgs& e)
{
}

void WorldClient::Controls::TextBoxHelper::SetTextLength(Noesis::DependencyObject* sender, std::function<int(Noesis::DependencyObject*)> funcTextLength)
{
}

void WorldClient::Controls::TextBoxHelper::TextChanged(Noesis::BaseComponent* sender, Noesis::RoutedEventArgs& e)
{
}

void WorldClient::Controls::TextBoxHelper::PasswordChanged(Noesis::BaseComponent* sender, Noesis::RoutedEventArgs& e)
{
}

void WorldClient::Controls::TextBoxHelper::OnDatePickerBaseSelectedDateChanged(Noesis::BaseComponent* sender, Noesis::RoutedEventArgs& e)
{
}

void WorldClient::Controls::TextBoxHelper::TextBoxGotFocus(Noesis::BaseComponent* sender, Noesis::RoutedEventArgs& e)
{
}

void WorldClient::Controls::TextBoxHelper::UIElementPreviewMouseLeftButtonDown(Noesis::BaseComponent* sender, Noesis::MouseButtonEventArgs& e)
{
}

void WorldClient::Controls::TextBoxHelper::PasswordGotFocus(Noesis::BaseComponent* sender, Noesis::RoutedEventArgs& e)
{
}

void WorldClient::Controls::TextBoxHelper::ControlGotFocus(Noesis::DependencyObject* sender, std::function<void(Noesis::DependencyObject*)> action)
{
}

bool WorldClient::Controls::TextBoxHelper::GetClearTextButton(Noesis::DependencyObject* d)
{
	return d->GetValue<bool>(ClearTextButtonProperty);
}

void WorldClient::Controls::TextBoxHelper::SetClearTextButton(Noesis::DependencyObject* obj, bool value)
{
	obj->SetValue<bool>(ClearTextButtonProperty, value);
}

bool WorldClient::Controls::TextBoxHelper::GetTextButton(Noesis::DependencyObject* d)
{
	return d->GetValue<bool>(TextButtonProperty);
}

void WorldClient::Controls::TextBoxHelper::SetTextButton(Noesis::DependencyObject* obj, bool value)
{
	obj->SetValue<bool>(TextButtonProperty, value);
}

ButtonsAlignment WorldClient::Controls::TextBoxHelper::GetButtonsAlignment(Noesis::DependencyObject* d)
{
	return d->GetValue<ButtonsAlignment>(ButtonsAlignmentProperty);
}

void WorldClient::Controls::TextBoxHelper::SetButtonsAlignment(Noesis::DependencyObject* obj, ButtonsAlignment value)
{
	obj->SetValue<ButtonsAlignment>(ButtonsAlignmentProperty, value);
}

bool WorldClient::Controls::TextBoxHelper::GetIsClearTextButtonBehaviorEnabled(Noesis::Button* d)
{
	return d->GetValue<bool>(IsClearTextButtonBehaviorEnabledProperty);
}

void WorldClient::Controls::TextBoxHelper::SetIsClearTextButtonBehaviorEnabled(Noesis::Button* obj, bool value)
{
	obj->SetValue<bool>(IsClearTextButtonBehaviorEnabledProperty, value);
}

double WorldClient::Controls::TextBoxHelper::GetButtonWidth(Noesis::DependencyObject* obj)
{
	return obj->GetValue<double>(ButtonWidthProperty);
}

void WorldClient::Controls::TextBoxHelper::SetButtonWidth(Noesis::DependencyObject* obj, double value)
{
	obj->SetValue<double>(ButtonWidthProperty, value);
}

Noesis::BaseCommand* WorldClient::Controls::TextBoxHelper::GetButtonCommand(Noesis::DependencyObject* d)
{
	return d->GetValue<Noesis::Ptr<Noesis::BaseCommand>>(ButtonCommandProperty).GetPtr();
}

void WorldClient::Controls::TextBoxHelper::SetButtonCommand(Noesis::DependencyObject* obj, Noesis::BaseCommand* value)
{
	obj->SetValue<Noesis::Ptr<Noesis::BaseCommand>>(ButtonCommandProperty, value);
}

Noesis::BaseComponent* WorldClient::Controls::TextBoxHelper::GetButtonCommandParameter(Noesis::DependencyObject* d)
{
	return d->GetValue<Noesis::Ptr<Noesis::BaseComponent>>(ButtonCommandParameterProperty).GetPtr();
}

void WorldClient::Controls::TextBoxHelper::SetButtonCommandParameter(Noesis::DependencyObject* obj, Noesis::BaseComponent* value)
{
	obj->SetValue<Noesis::Ptr<Noesis::BaseComponent>>(ButtonCommandParameterProperty, value);
}

Noesis::BaseComponent* WorldClient::Controls::TextBoxHelper::GetButtonContent(Noesis::DependencyObject* d)
{
	return d->GetValue<Noesis::Ptr<Noesis::BaseComponent>>(ButtonContentProperty).GetPtr();
}

void WorldClient::Controls::TextBoxHelper::SetButtonContent(Noesis::DependencyObject* obj, Noesis::BaseComponent* value)
{
	obj->SetValue<Noesis::Ptr<Noesis::BaseComponent>>(ButtonContentProperty, value);
}

Noesis::DataTemplate* WorldClient::Controls::TextBoxHelper::GetButtonContentTemplate(Noesis::DependencyObject* d)
{
	return d->GetValue<Noesis::Ptr<Noesis::DataTemplate>>(ButtonContentTemplateProperty).GetPtr();
}

void WorldClient::Controls::TextBoxHelper::SetButtonContentTemplate(Noesis::DependencyObject* obj, Noesis::DataTemplate* value)
{
	obj->SetValue<Noesis::Ptr<Noesis::DataTemplate>>(ButtonContentTemplateProperty, value);
}

Noesis::ControlTemplate* WorldClient::Controls::TextBoxHelper::GetButtonTemplate(Noesis::DependencyObject* d)
{
	return d->GetValue<Noesis::Ptr<Noesis::ControlTemplate>>(ButtonTemplateProperty).GetPtr();
}

void WorldClient::Controls::TextBoxHelper::SetButtonTemplate(Noesis::DependencyObject* obj, Noesis::ControlTemplate* value)
{
	obj->SetValue<Noesis::Ptr<Noesis::ControlTemplate>>(ButtonTemplateProperty, value);
}

Noesis::FontFamily* WorldClient::Controls::TextBoxHelper::GetButtonFontFamily(Noesis::DependencyObject* d)
{
	return d->GetValue<Noesis::Ptr<Noesis::FontFamily>>(ButtonFontFamilyProperty);
}

void WorldClient::Controls::TextBoxHelper::SetButtonFontFamily(Noesis::DependencyObject* obj, Noesis::FontFamily* value)
{
	obj->SetValue<Noesis::Ptr<Noesis::FontFamily>>(ButtonFontFamilyProperty, value);
}

double WorldClient::Controls::TextBoxHelper::GetButtonFontSize(Noesis::DependencyObject* d)
{
	return d->GetValue<double>(ButtonFontSizeProperty);
}

void WorldClient::Controls::TextBoxHelper::SetButtonFontSize(Noesis::DependencyObject* obj, double value)
{
	obj->SetValue<double>(ButtonFontSizeProperty, value);
}

void WorldClient::Controls::TextBoxHelper::ButtonClicked(Noesis::BaseComponent* sender, Noesis::RoutedEventArgs* e)
{
}

void WorldClient::Controls::TextBoxHelper::IsClearTextButtonBehaviorEnabledChanged(Noesis::DependencyObject* d, const Noesis::DependencyPropertyChangedEventArgs& e)
{
}

void WorldClient::Controls::TextBoxHelper::ButtonCommandOrClearTextChanged(Noesis::DependencyObject* d, const Noesis::DependencyPropertyChangedEventArgs& e)
{
}

void WorldClient::Controls::TextBoxHelper::RichTextBoxLoaded(Noesis::BaseComponent* sender, Noesis::RoutedEventArgs& e)
{
}

void WorldClient::Controls::TextBoxHelper::ComboBoxLoaded(Noesis::BaseComponent* sender, Noesis::RoutedEventArgs& e)
{
}

const Noesis::DependencyProperty* TextBoxHelper::IsMonitoringProperty;
const Noesis::DependencyProperty* TextBoxHelper::WatermarkProperty;
const Noesis::DependencyProperty* TextBoxHelper::WatermarkAlignmentProperty;
const Noesis::DependencyProperty* TextBoxHelper::WatermarkTrimmingProperty;
const Noesis::DependencyProperty* TextBoxHelper::WatermarkWrappingProperty;
const Noesis::DependencyProperty* TextBoxHelper::UseFloatingWatermarkProperty;
const Noesis::DependencyProperty* TextBoxHelper::TextLengthProperty;
const Noesis::DependencyProperty* TextBoxHelper::ClearTextButtonProperty;
const Noesis::DependencyProperty* TextBoxHelper::TextButtonProperty;
const Noesis::DependencyProperty* TextBoxHelper::ButtonsAlignmentProperty;
const Noesis::DependencyProperty* TextBoxHelper::IsClearTextButtonBehaviorEnabledProperty;
const Noesis::DependencyProperty* TextBoxHelper::ButtonWidthProperty;
const Noesis::DependencyProperty* TextBoxHelper::ButtonCommandProperty;
const Noesis::DependencyProperty* TextBoxHelper::ButtonCommandParameterProperty;
const Noesis::DependencyProperty* TextBoxHelper::ButtonContentProperty;
const Noesis::DependencyProperty* TextBoxHelper::ButtonContentTemplateProperty;
const Noesis::DependencyProperty* TextBoxHelper::ButtonTemplateProperty;
const Noesis::DependencyProperty* TextBoxHelper::ButtonFontFamilyProperty;
const Noesis::DependencyProperty* TextBoxHelper::ButtonFontSizeProperty;
const Noesis::DependencyProperty* TextBoxHelper::SelectAllOnFocusProperty;
const Noesis::DependencyProperty* TextBoxHelper::IsWaitingForDataProperty;
const Noesis::DependencyProperty* TextBoxHelper::HasTextProperty;
const Noesis::DependencyProperty* TextBoxHelper::IsSpellCheckContextMenuEnabledProperty;
const Noesis::DependencyProperty* TextBoxHelper::AutoWatermarkProperty;

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION_ENUM(WorldClient::Controls::ButtonsAlignment)
{
	NsVal("Left", WorldClient::Controls::ButtonsAlignment::Left);
	NsVal("Opposite", WorldClient::Controls::ButtonsAlignment::Opposite);
	NsVal("Right", WorldClient::Controls::ButtonsAlignment::Right);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION_ENUM(WorldClient::Controls::SpellingResourceKeyId)
{
	NsVal("IgnoreAllMenuItemStyle", WorldClient::Controls::SpellingResourceKeyId::IgnoreAllMenuItemStyle);
	NsVal("NoSuggestionsMenuItemStyle", WorldClient::Controls::SpellingResourceKeyId::NoSuggestionsMenuItemStyle);
	NsVal("SeparatorStyle", WorldClient::Controls::SpellingResourceKeyId::SeparatorStyle);
	NsVal("SuggestionMenuItemStyle", WorldClient::Controls::SpellingResourceKeyId::SuggestionMenuItemStyle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(TextChangedEventArgs)
{
	NsMeta<Noesis::TypeId>("WorldClient.Controls.TextChangedEventArgs");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(TextBoxHelper)
{
	NsMeta<TypeId>("WorldClient.Controls.TextBoxHelper");

	UIElementData* data = NsMeta<UIElementData>(TypeOf<SelfClass>());
	data->RegisterProperty<bool>(IsMonitoringProperty, "IsMonitoring",
		PropertyMetadata::Create(false));
	data->RegisterProperty<eastl::string>(WatermarkProperty, "Watermark",
		PropertyMetadata::Create(eastl::string("")));
    data->RegisterProperty<Noesis::TextAlignment>(WatermarkAlignmentProperty, "WatermarkAlignment",
        PropertyMetadata::Create(Noesis::TextAlignment::TextAlignment_Left));
    data->RegisterProperty<Noesis::TextTrimming>(WatermarkTrimmingProperty, "WatermarkTrimming",
		PropertyMetadata::Create(Noesis::TextTrimming::TextTrimming_None));
	data->RegisterProperty<Noesis::TextWrapping>(WatermarkWrappingProperty, "WatermarkWrapping",
		PropertyMetadata::Create(Noesis::TextWrapping::TextWrapping_NoWrap, Noesis::PropertyChangedCallback(ButtonCommandOrClearTextChanged)));
	data->RegisterProperty<bool>(UseFloatingWatermarkProperty, "UseFloatingWatermark",
		PropertyMetadata::Create(false, Noesis::PropertyChangedCallback(ButtonCommandOrClearTextChanged)));
	data->RegisterProperty<int32_t>(TextLengthProperty, "TextLength",
		PropertyMetadata::Create(0));
	data->RegisterProperty<bool>(ClearTextButtonProperty, "ClearTextButton",
		PropertyMetadata::Create(false));
	data->RegisterProperty<bool>(TextButtonProperty, "TextButton",
		PropertyMetadata::Create(false));
	data->RegisterProperty<ButtonsAlignment>(ButtonsAlignmentProperty, "ButtonsAlignment",
		PropertyMetadata::Create(ButtonsAlignment::Left));
	data->RegisterProperty<bool>(IsClearTextButtonBehaviorEnabledProperty, "IsClearTextButtonBehaviorEnabled",
		PropertyMetadata::Create(false));
	data->RegisterProperty<double>(ButtonWidthProperty, "ButtonWidth",
		PropertyMetadata::Create(22.0));
	data->RegisterProperty<Noesis::Ptr<Noesis::BaseCommand>>(ButtonCommandProperty, "ButtonCommand",
		PropertyMetadata::Create(Noesis::Ptr<Noesis::BaseCommand>()));
	data->RegisterProperty<Noesis::Ptr<Noesis::BaseComponent>>(ButtonCommandParameterProperty, "ButtonCommandParameter",
		PropertyMetadata::Create(Noesis::Ptr<Noesis::BaseComponent>()));
	data->RegisterProperty<Noesis::Ptr<Noesis::BaseComponent>>(ButtonContentProperty, "ButtonContent",
		PropertyMetadata::Create(Noesis::Ptr<Noesis::BaseComponent>()));
	data->RegisterProperty<Noesis::Ptr<Noesis::DataTemplate>>(ButtonContentTemplateProperty, "ButtonContentTemplate",
		PropertyMetadata::Create(Noesis::Ptr<Noesis::DataTemplate>()));
	data->RegisterProperty<Noesis::Ptr<Noesis::ControlTemplate>>(ButtonTemplateProperty, "ButtonTemplate",
		PropertyMetadata::Create(Noesis::Ptr<Noesis::ControlTemplate>()));
	data->RegisterProperty<Noesis::Ptr<Noesis::FontFamily>>(ButtonFontFamilyProperty, "ButtonFontFamily",
		PropertyMetadata::Create(Noesis::Ptr<Noesis::FontFamily>()));
	data->RegisterProperty<double>(ButtonFontSizeProperty, "ButtonFontSize",
		PropertyMetadata::Create(12.0));
	data->RegisterProperty<bool>(IsWaitingForDataProperty, "IsWaitingForData",
		PropertyMetadata::Create(false));
	data->RegisterProperty<bool>(HasTextProperty, "HasText",
		PropertyMetadata::Create(false));
	data->RegisterProperty<bool>(IsSpellCheckContextMenuEnabledProperty, "IsSpellCheckContextMenuEnabled",
		PropertyMetadata::Create(false));
	data->RegisterProperty<bool>(AutoWatermarkProperty, "AutoWatermark",
		PropertyMetadata::Create(false));
}