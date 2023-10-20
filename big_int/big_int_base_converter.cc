/** 
 *  @file   big_int_base_converter.cc
 *  @brief  Source file for the big int base converter class
 *
 *  This file contains the source code for the big int base converter class.
 *  This code is taken from: https://stackoverflow.com/questions/4735622/convert-large-hex-string-to-decimal-string
 *  [Convert large hex string to decimal string]
 *
 *  @bug            No known bugs.
 */

#include <stdexcept>
#include <algorithm>

#include "big_int_base_converter.hpp"

BaseConverter::BaseConverter(const char *sourceBaseSet, const char *targetBaseSet) 
    : sourceBaseSet_(sourceBaseSet)
    , targetBaseSet_(targetBaseSet) {

    if (sourceBaseSet == NULL || targetBaseSet == NULL) {
        throw std::invalid_argument("Invalid base character set");
    }

}

std::string BaseConverter::Convert(std::string value) const {

    unsigned int numberBase = GetTargetBase();
    std::string result;

    do {
    
        unsigned int remainder = divide(sourceBaseSet_, value, numberBase);
        result.push_back(targetBaseSet_[remainder]);
    
    } while (!value.empty() && !(value.length() == 1 && value[0] == sourceBaseSet_[0]));

    std::reverse(result.begin(), result.end());
    return result;

}

unsigned int BaseConverter::divide(const std::string& baseDigits, std::string& x, unsigned int y) {

    std::string quotient;

    size_t lenght = x.length();

    for (size_t i = 0; i < lenght; ++i) {
        size_t j = i + 1 + x.length() - lenght;
        if (x.length() < j)
            break;

        unsigned int value = base2dec(baseDigits, x.substr(0, j));

        quotient.push_back(baseDigits[value / y]);
        x = dec2base(baseDigits, value % y) + x.substr(j);
    }

    // calculate remainder
    unsigned int remainder = base2dec(baseDigits, x);

    // remove leading "zeros" from quotient and store in 'x'
    size_t n = quotient.find_first_not_of(baseDigits[0]);
    if (n != std::string::npos) {
        x = quotient.substr(n);
    } else {
        x.clear();
    }

    return remainder;
}

std::string BaseConverter::dec2base(const std::string& baseDigits, unsigned int value) {

    unsigned int numberBase = (unsigned int)baseDigits.length();
    std::string result;
    do {

        result.push_back(baseDigits[value % numberBase]);
        value /= numberBase;

    } while (value > 0);

    std::reverse(result.begin(), result.end());
    return result;
}

unsigned int BaseConverter::base2dec(const std::string& baseDigits, const std::string& value) {

    unsigned int numberBase = (unsigned int)baseDigits.length();
    unsigned int result = 0;
    for (size_t i = 0; i < value.length(); ++i) {

        result *= numberBase;
        int c = baseDigits.find(value[i]);
        if (c == std::string::npos)
            throw std::runtime_error("Invalid character");

        result += (unsigned int)c;
    }

    return result;
}
