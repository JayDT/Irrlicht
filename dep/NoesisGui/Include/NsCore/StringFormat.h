////////////////////////////////////////////////////////////////////////////////////////////////////
// NoesisGUI - http://www.noesisengine.com
// Copyright (c) 2013 Noesis Technologies S.L. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef __GUI_STRINGFORMAT_H__
#define __GUI_STRINGFORMAT_H__


#include <NsCore/Noesis.h>
#include <NsCore/KernelApi.h>
#include <NsCore/NSTLForwards.h>


namespace Noesis
{

class BaseComponent;

// Provides culture-specific information for formatting and parsing numeric values.
struct NumberFormatInfo
{
    // String that denotes that the associated number is positive
    const char* positiveSign = "+";

    // String that denotes that the associated number is negative
    const char* negativeSign = "-";

    // Format pattern for negative numeric values
    // (n) -n - n n- n -
    int numberNegativePattern = 1;

    // Number of decimal places to use in numeric values
    int numberDecimalDigits = 2;

    // String to use as the decimal separator in numeric values
    const char* numberDecimalSeparator = ".";

    // String that separates groups of digits to the left of the decimal in numeric values
    const char* numberGroupSeparator = ",";

    // Number of decimal places to use in currency values
    int currencyDecimalDigits = 2;

    // Format pattern for positive currency values
    // '$n' 'n$' '$ n' 'n $'
    int currencyPositivePattern = 0;
    
    // Format pattern for negative currency values
    // '($n)' '-$n' '$-n' '$n-' '(n$)' '-n$' 'n-$' 'n$-' '-n $' '-$ n' 'n $-' '$ n-' '$ -n' 'n- $' '($ n)' '(n $)'
    int currencyNegativePattern = 0;
    
    // String to use as the currency symbol
    const char* currencySymbol = "$";

    // String to use as the decimal separator in currency values
    const char* currencyDecimalSeparator = ".";

    // String that separates groups of digits to the left of the decimal in currency values
    const char* currencyGroupSeparator = ",";

    // Number of decimal places to use in percent values
    int percentDecimalDigits = 2;

    // Format pattern for positive percent values
    // 'n %' 'n%' '%n' '% n'
    int percentPositivePattern = 0;

    // Format pattern for negative percent values
    // '-n %' '-n%' '-%n' '%-n' '%n-' 'n-%' 'n%-' '-% n' 'n %-' '% n-' '% -n' 'n- %'
    int percentNegativePattern = 0;

    // String to use as the percent symbol
    const char* percentSymbol = "%";

    // String to use as the decimal separator in percent values
    const char* percentDecimalSeparator = ".";

    // String that separates groups of digits to the left of the decimal in percent values
    const char* percentGroupSeparator = ",";

    // String that represents negative infinity
    const char* negativeInfinitySymbol = "-\xE2\x88\x9E";

    // String that represents positive infinity
    const char* positiveInfinitySymbol = "\xE2\x88\x9E";

    // String that represents the IEEE NaN (not a number) value
    const char* nanSymbol = "NaN";
};

/// String formatting using C# style ("F4", "###.0", "######e0")
NS_CORE_KERNEL_API void StringFormat(BaseComponent* value, const char* format,
    const NumberFormatInfo& culture, NsFixedString<128>& str);

/// String formatting using default number format information
NS_CORE_KERNEL_API void StringFormat(BaseComponent* value, const char* format,
    NsFixedString<128>& str);

/// Float formatting using general ("G") format
NS_CORE_KERNEL_API void StringFormat(float value, NsFixedString<128>& str);

}

#endif
