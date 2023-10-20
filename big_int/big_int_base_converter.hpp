/** 
 *  @file   big_int_base_converter.hpp
 *  @brief  Header file for the big int base converter class
 *
 *  This file contains the Header code for the big int base converter class
 *  This code is taken from: https://stackoverflow.com/questions/4735622/convert-large-hex-string-to-decimal-string
 *  [Convert large hex string to decimal string]
 *
 *  @bug            No known bugs.
 */

#pragma once

#include <string>

class BaseConverter {

public:

    BaseConverter(const char *sourceBaseSet, const char *targetBaseSet);
    std::string GetSourceBaseSet() const { return sourceBaseSet_; }
    std::string GetTargetBaseSet() const { return targetBaseSet_; }
    unsigned int GetSourceBase() const { return (unsigned int)sourceBaseSet_.length(); }
    unsigned int GetTargetBase() const { return (unsigned int)targetBaseSet_.length(); }

    std::string  Convert(std::string value) const;


private:

    /* Divides x by y, and returns the quotient and remainder. */
    static unsigned int divide(const std::string& baseDigits, 
                               std::string& x, 
                               unsigned int y);

    static unsigned int base2dec(const std::string& baseDigits,
                                 const std::string& value);

    static std::string dec2base(const std::string& baseDigits, unsigned int value);

private:

    std::string         sourceBaseSet_;
    std::string         targetBaseSet_;

};
