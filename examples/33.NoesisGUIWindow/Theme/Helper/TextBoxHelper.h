#ifndef __WC_TEXTBOXHELPER_H__
#define __WC_TEXTBOXHELPER_H__

#include <NsCore/Noesis.h>
#include <NsCore/ReflectionDeclare.h>
#include <NsGui/DependencyObject.h>

namespace WorldClient::Controls
{
	enum class ButtonsAlignment
	{
		Left,
		Right,
		Opposite
	};


	enum class SpellingResourceKeyId
	{
		SuggestionMenuItemStyle,
		IgnoreAllMenuItemStyle,
		NoSuggestionsMenuItemStyle,
		SeparatorStyle,
	};

	//class Spelling final
	//{
	//private:
	//    static Noesis::ResourceKey *m_SuggestionMenuItemStyleKey;
	//    static Noesis::ResourceKey *m_IgnoreAllMenuItemStyleKey;
	//    static Noesis::ResourceKey *m_NoSuggestionsMenuItemStyleKey;
	//    static Noesis::ResourceKey *m_SeparatorStyleKey;
	//
	//public:
	//    static Noesis::ResourceKey *GetSuggestionMenuItemStyleKey();
	//    static Noesis::ResourceKey *GetIgnoreAllMenuItemStyleKey();
	//    static Noesis::ResourceKey *GetNoSuggestionsMenuItemStyleKey();
	//    static Noesis::ResourceKey *GetSeparatorStyleKey();
	//};

	class TextChangedEventArgs final : public Noesis::BaseComponent
	{
	public:
		TextChangedEventArgs() {}

		TextChangedEventArgs(const eastl::string& text)
			: mText(text)
		{}

		const eastl::string mText;
	private:
		const Noesis::PropertyMetadata* metadata;

		NS_DECLARE_REFLECTION(TextChangedEventArgs, Noesis::NoParent)
	};

	/// <summary>
	/// A helper class that provides various attached properties for the TextBox control.
	/// </summary>
	/// <remarks>
	/// Password watermarking code from: http://prabu-guru.blogspot.com/2010/06/how-to-add-watermark-text-to-textbox.html
	/// </remarks>
	class TextBoxHelper final : public Noesis::DependencyObject
	{
	public:
		static const Noesis::DependencyProperty* IsMonitoringProperty;
		static const Noesis::DependencyProperty* WatermarkProperty;
		static const Noesis::DependencyProperty* WatermarkAlignmentProperty;
		static const Noesis::DependencyProperty* WatermarkTrimmingProperty;
		static const Noesis::DependencyProperty* WatermarkWrappingProperty;
		static const Noesis::DependencyProperty* UseFloatingWatermarkProperty;
		static const Noesis::DependencyProperty* TextLengthProperty;
		static const Noesis::DependencyProperty* ClearTextButtonProperty;
		static const Noesis::DependencyProperty* TextButtonProperty;
		static const Noesis::DependencyProperty* ButtonsAlignmentProperty;
		static const Noesis::DependencyProperty* IsClearTextButtonBehaviorEnabledProperty;
		static const Noesis::DependencyProperty* ButtonWidthProperty;
		static const Noesis::DependencyProperty* ButtonCommandProperty;
		static const Noesis::DependencyProperty* ButtonCommandParameterProperty;
		static const Noesis::DependencyProperty* ButtonContentProperty;
		static const Noesis::DependencyProperty* ButtonContentTemplateProperty;
		static const Noesis::DependencyProperty* ButtonTemplateProperty;
		static const Noesis::DependencyProperty* ButtonFontFamilyProperty;
		static const Noesis::DependencyProperty* ButtonFontSizeProperty;
		static const Noesis::DependencyProperty* SelectAllOnFocusProperty;
		static const Noesis::DependencyProperty* IsWaitingForDataProperty;
		static const Noesis::DependencyProperty* HasTextProperty;
		static const Noesis::DependencyProperty* IsSpellCheckContextMenuEnabledProperty;
		static const Noesis::DependencyProperty* AutoWatermarkProperty;

	private:
		static const std::unordered_map<std::type_info, Noesis::Ptr<Noesis::DependencyProperty>> AutoWatermarkPropertyMapping;

	public:
		/// <summary>
		/// Indicates if a TextBox or RichTextBox should use SpellCheck context menu
		/// </summary>
		static bool GetIsSpellCheckContextMenuEnabled(Noesis::UIElement* element);
		static void SetIsSpellCheckContextMenuEnabled(Noesis::UIElement* element, bool value);

		static bool GetAutoWatermark(Noesis::DependencyObject* element);

		///  <summary>
		///  Indicates if the watermark is automatically retrieved by using the <see cref="DisplayAttribute"/> of the bound property.
		///  </summary>
		/// <remarks>This attached property uses reflection; thus it might reduce the performance of the application.
		/// The auto-watermak does work for the following controls:
		/// In the following case no custom watermark is shown
		/// <list type="bullet">
		/// <item>There is no binding</item>
		/// <item>Binding path errors</item>
		/// <item>Binding to a element of a collection without using a property of that element <c>Binding Path=Collection[0]</c> use: <c>Binding Path=Collection[0].SubProperty</c></item>
		/// <item>The bound property does not have a <see cref="DisplayAttribute"/></item>
		/// </list></remarks>
		static void SetAutoWatermark(Noesis::DependencyObject* element, bool value);

	private:
		static void OnAutoWatermarkChanged(Noesis::DependencyObject* d, Noesis::DependencyPropertyChangedEventArgs& e);
		static void OnControlWithAutoWatermarkSupportLoaded(Noesis::BaseComponent* o, Noesis::RoutedEventArgs& routedEventArgs);
		//static Noesis::PropertyInfo* ResolvePropertyFromBindingExpression(Noesis::BindingExpression* bindingExpression);

		static void IsSpellCheckContextMenuEnabledChanged(Noesis::DependencyObject* d, Noesis::DependencyPropertyChangedEventArgs& e);
		static void TextBoxBaseLostFocus(Noesis::BaseComponent* sender, Noesis::RoutedEventArgs& e);
		static void TextBoxBaseContextMenuClosing(Noesis::BaseComponent* sender, Noesis::ContextMenuEventArgs& e);
		static void TextBoxBaseContextMenuOpening(Noesis::BaseComponent* sender, Noesis::ContextMenuEventArgs& e);
		static void RemoveSpellCheckMenuItems(Noesis::ContextMenu* contextMenu);

	public:
		static void SetIsWaitingForData(Noesis::DependencyObject* obj, bool value);
		static bool GetIsWaitingForData(Noesis::DependencyObject* obj);

		static void SetSelectAllOnFocus(Noesis::DependencyObject* obj, bool value);
		static bool GetSelectAllOnFocus(Noesis::DependencyObject* obj);

		static void SetIsMonitoring(Noesis::DependencyObject* obj, bool value);
		static const char* GetWatermark(Noesis::DependencyObject* obj);

		static void SetWatermark(Noesis::DependencyObject* obj, const char* value);

		/// <summary>
		/// Gets a value that indicates the horizontal alignment of the watermark.
		/// </summary>
		/// <returns>
		/// One of the <see cref="System.Windows.TextAlignment" /> values that specifies the desired alignment. The default is <see cref="System.Windows.TextAlignment.Left" />.
		/// </returns>
		static Noesis::TextAlignment GetWatermarkAlignment(DependencyObject* obj);

		/// <summary>
		/// Sets a value that indicates the horizontal alignment of the watermark.
		/// </summary>
		static void SetWatermarkAlignment(DependencyObject* obj, Noesis::TextAlignment value);

		/// <summary>
		/// Gets the text trimming behavior to employ when watermark overflows the content area.
		/// </summary>
		/// <returns>
		/// One of the <see cref="T:System.Windows.TextTrimming" /> values that specifies the text trimming behavior to employ. The default is <see cref="F:System.Windows.TextTrimming.None" />.
		/// </returns>
		static Noesis::TextTrimming GetWatermarkTrimming(DependencyObject* obj);

		/// <summary>
		/// Sets the text trimming behavior to employ when watermark overflows the content area.
		/// </summary>
		static void SetWatermarkTrimming(DependencyObject* obj, Noesis::TextTrimming value);

		/// <summary>
		/// Gets how the watermark should wrap text.
		/// </summary>
		/// <returns>One of the <see cref="T:System.Windows.TextWrapping" /> values. The default is <see cref="F:System.Windows.TextWrapping.NoWrap" />.
		/// </returns>
		static Noesis::TextWrapping GetWatermarkWrapping(DependencyObject* obj);

		/// <summary>
		/// Sets how the watermark should wrap text.
		/// </summary>
		static void SetWatermarkWrapping(Noesis::DependencyObject* obj, Noesis::TextWrapping value);

		static bool GetUseFloatingWatermark(Noesis::DependencyObject* obj);
		static void SetUseFloatingWatermark(Noesis::DependencyObject* obj, bool value);

		/// <summary>
		/// Gets if the attached TextBox has text.
		/// </summary>
		static bool GetHasText(Noesis::DependencyObject* obj);
		static void SetHasText(Noesis::DependencyObject* obj, bool value);

		static int32_t GetTextLength(Noesis::DependencyObject* obj);
		static void SetTextLength(Noesis::DependencyObject* obj, int32_t value);

		static void OnIsMonitoringChanged(Noesis::DependencyObject* d, Noesis::DependencyPropertyChangedEventArgs& e);

	private:
		static void RichTextBox_TextChanged(Noesis::BaseComponent* sender, TextChangedEventArgs& e);

		static void SetTextLength(Noesis::DependencyObject* sender, std::function<int(Noesis::DependencyObject*)> funcTextLength);

		static void TextChanged(Noesis::BaseComponent* sender, Noesis::RoutedEventArgs& e);

		static void PasswordChanged(Noesis::BaseComponent* sender, Noesis::RoutedEventArgs& e);

		static void OnDatePickerBaseSelectedDateChanged(Noesis::BaseComponent* sender, Noesis::RoutedEventArgs& e);

		static void TextBoxGotFocus(Noesis::BaseComponent* sender, Noesis::RoutedEventArgs& e);

		static void UIElementPreviewMouseLeftButtonDown(Noesis::BaseComponent* sender, Noesis::MouseButtonEventArgs& e);

		static void PasswordGotFocus(Noesis::BaseComponent* sender, Noesis::RoutedEventArgs& e);

		static void ControlGotFocus(Noesis::DependencyObject* sender, std::function<void(Noesis::DependencyObject*)> action);

	public:
		/// <summary>
		/// Gets the clear text button visibility / feature. Can be used to enable text deletion.
		/// </summary>
		static bool GetClearTextButton(Noesis::DependencyObject* d);

		/// <summary>
		/// Sets the clear text button visibility / feature. Can be used to enable text deletion.
		/// </summary>
		static void SetClearTextButton(Noesis::DependencyObject* obj, bool value);

		/// <summary>
		/// Gets the text button visibility.
		/// </summary>
		static bool GetTextButton(Noesis::DependencyObject* d);

		/// <summary>
		/// Sets the text button visibility.
		/// </summary>
		static void SetTextButton(Noesis::DependencyObject* obj, bool value);

		/// <summary>
		/// Gets the buttons placement variant.
		/// </summary>
		static ButtonsAlignment GetButtonsAlignment(Noesis::DependencyObject* d);

		/// <summary>
		/// Sets the buttons placement variant.
		/// </summary>
		static void SetButtonsAlignment(Noesis::DependencyObject* obj, ButtonsAlignment value);

		/// <summary>
		/// Gets the clear text button behavior.
		/// </summary>
		static bool GetIsClearTextButtonBehaviorEnabled(Noesis::Button* d);

		/// <summary>
		/// Sets the clear text button behavior.
		/// </summary>
		static void SetIsClearTextButtonBehaviorEnabled(Noesis::Button* obj, bool value);

		static double GetButtonWidth(Noesis::DependencyObject* obj);
		static void SetButtonWidth(Noesis::DependencyObject* obj, double value);

		static Noesis::BaseCommand* GetButtonCommand(Noesis::DependencyObject* d);
		static void SetButtonCommand(Noesis::DependencyObject* obj, Noesis::BaseCommand* value);

		static Noesis::BaseComponent* GetButtonCommandParameter(Noesis::DependencyObject* d);
		static void SetButtonCommandParameter(Noesis::DependencyObject* obj, Noesis::BaseComponent* value);

		static Noesis::BaseComponent* GetButtonContent(Noesis::DependencyObject* d);
		static void SetButtonContent(Noesis::DependencyObject* obj, Noesis::BaseComponent* value);

		/// <summary> 
		/// ButtonContentTemplate is the template used to display the content of the ClearText button. 
		/// </summary>
		static Noesis::DataTemplate* GetButtonContentTemplate(Noesis::DependencyObject* d);
		static void SetButtonContentTemplate(Noesis::DependencyObject* obj, Noesis::DataTemplate* value);

		static Noesis::ControlTemplate* GetButtonTemplate(Noesis::DependencyObject* d);
		static void SetButtonTemplate(Noesis::DependencyObject* obj, Noesis::ControlTemplate* value);

		static Noesis::FontFamily* GetButtonFontFamily(Noesis::DependencyObject* d);
		static void SetButtonFontFamily(Noesis::DependencyObject* obj, Noesis::FontFamily* value);

		static double GetButtonFontSize(Noesis::DependencyObject* d);
		static void SetButtonFontSize(Noesis::DependencyObject* obj, double value);

	public:
		static void ButtonClicked(Noesis::BaseComponent* sender, Noesis::RoutedEventArgs* e);

	private:
		static void IsClearTextButtonBehaviorEnabledChanged(Noesis::DependencyObject* d, const Noesis::DependencyPropertyChangedEventArgs& e);

		static void ButtonCommandOrClearTextChanged(Noesis::DependencyObject* d, const Noesis::DependencyPropertyChangedEventArgs& e);

		static void RichTextBoxLoaded(Noesis::BaseComponent* sender, Noesis::RoutedEventArgs& e);

		//static void SetRichTextBoxTextLength(Noesis::RichTextBox* richTextBox);

		static void ComboBoxLoaded(Noesis::BaseComponent* sender, Noesis::RoutedEventArgs& e);

	private:
		NS_DECLARE_REFLECTION(TextBoxHelper, Noesis::BaseComponent)
	};
}

NS_DECLARE_REFLECTION_ENUM(WorldClient::Controls::ButtonsAlignment)
NS_DECLARE_REFLECTION_ENUM(WorldClient::Controls::SpellingResourceKeyId)

#endif	//#ifndef __WC_TEXTBOXHELPER_H__
