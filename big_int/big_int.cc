/** 
 *  @file   big_int.cc
 *  @brief  Main source file for the big int library
 *
 *  This file contains the source code and the DS used in the big int library
 *
 *  @author         Tony Josi   https://tonyjosi97.github.io/profile/
 *  @copyright      Copyright (C) 2021 Tony Josi
 *  @bug            No known bugs.
 */

#include <stdexcept>
#include <algorithm>
#include <string.h>
#include <memory>
#include <cstdio>
#include <random>
#include <iostream>
#include <mutex>
#include <atomic>
#include <vector>
#include <thread>

#include "big_int.hpp"
#include "big_int_lib_log.hpp"
#include "big_int_inline_defs.hpp"
#include "big_int_base_converter.hpp"

const char *bin_num_set = "01";
const char *dec_num_set = "0123456789";
const char *hex_num_set = "0123456789ABCDEF";

int bi::big_int::big_int_from_string(const std::string &str_num_arg, bi_base target_base) {

    std::string str_num(str_num_arg);

    bool is_neg = false;
    if (str_num[0] == '-') {
        is_neg = true;
        str_num = str_num.substr(1);
    }

    size_t n = str_num.find_first_not_of('0');
    if (n != std::string::npos) {
        str_num = str_num.substr(n);
    }
    else {
        str_num = "0";
    }

    std::string hex_str;
    switch (target_base) {
        case bi_base::BI_BIN: {
            BaseConverter bin_to_hex(bin_num_set, hex_num_set);
            hex_str = bin_to_hex.Convert(str_num);
            break;
        }
        case bi_base::BI_DEC: {
            BaseConverter dec_to_hex(dec_num_set, hex_num_set);
            hex_str = dec_to_hex.Convert(str_num);
            break;
        }
        case bi_base::BI_HEX:
            hex_str = str_num;
            break;
        default:
            throw std::invalid_argument("Invalid base");
    }

    if (is_neg) {
        hex_str.insert(0, "-");
    }

    return _big_int_from_string(hex_str);;
}

int bi::big_int::big_int_from_base_type(const BI_BASE_TYPE &bt_val, const bool is_neg) {

    big_int_clear();
    _data[_top++] = bt_val;
    _neg = is_neg;
    return 0;

}

int bi::big_int::big_int_unsigned_add(const bi::big_int &b) {

    int max_data_len, min_data_len;

    if (_top > b._top) {
        min_data_len = b._top;
        max_data_len = _top;
    } else {
        min_data_len = _top;
        max_data_len = b._top;
    }

    int i;

    BI_DOUBLE_BASE_TYPE sum = 0;
    BI_BASE_TYPE carry = 0;
    for(i = 0; i < min_data_len; ++i) {
        sum = static_cast<BI_DOUBLE_BASE_TYPE>(b._data[i]) + _data[i] + carry;
        carry = static_cast<BI_BASE_TYPE>((sum) >> BI_BASE_TYPE_TOTAL_BITS);
        _data[i] = sum & BI_BASE_TYPE_MAX;
    }

    if(max_data_len >= _total_data) {
        _big_int_expand(BI_DEFAULT_EXPAND_COUNT + max_data_len);
    }
    
    int top_cntr = 0;
    for(; i < max_data_len; ++i) {
        if(i >= _top) {
            ++top_cntr;
        }
        if(i < _top) {
            sum = static_cast<BI_DOUBLE_BASE_TYPE>(_data[i]) + carry;
        } else {
            sum = static_cast<BI_DOUBLE_BASE_TYPE>(b._data[i]) + carry;
        }
        carry = static_cast<BI_BASE_TYPE>((sum) >> BI_BASE_TYPE_TOTAL_BITS);
        _data[i] = sum & BI_BASE_TYPE_MAX;
    }
    _top += top_cntr;
    
    if (carry) {
        if(i >= _total_data) {
            _big_int_expand(BI_DEFAULT_EXPAND_COUNT);
        }
        _data[_top++] = carry;
    }

    return 0;

}

int bi::big_int::big_int_unsigned_add(const bi::big_int &b, bi::big_int &res) {

    res.big_int_clear();
    res.big_int_unsigned_add(b);
    res.big_int_unsigned_add(*this);
    return 0;

}

int bi::big_int::big_int_signed_add(const bi::big_int &b) {

    bi::big_int temp;
    big_int_signed_add(b, temp);
    _big_int_swap(temp);
    return 0;

}

int bi::big_int::big_int_signed_add(const bi::big_int &b, bi::big_int &res) {

    res.big_int_clear();
    /* If the operands are of same sign. */
    if (_neg == b._neg) {

        big_int_unsigned_add(b, res);
        res._neg = _neg;

    }
    /* If operands are of different sign. */
    else {

        /* Convert to same type. */
        _neg = !_neg;
        int comp_stat = big_int_compare(b);
        if (comp_stat == 1) {
            if (_neg) {
                b.big_int_unsigned_sub(*this, res);
                res._neg = b._neg;
            } else {
                big_int_unsigned_sub(b, res);
                res._neg = !_neg;
            }
        } else if (comp_stat == 0) {
            res.big_int_set_zero();
        } else {
            if (_neg) {
                big_int_unsigned_sub(b, res);
                res._neg = !_neg;
            } else {
                b.big_int_unsigned_sub(*this, res);
                res._neg = b._neg;
            }
        }

        /* Set back to previous type. */
        _neg = !_neg;

    }

    /* Remove any preceding zeroes if any. */
    res._big_int_remove_preceding_zeroes();

    return 0;

}

int bi::big_int::big_int_left_shift_word(int shift_words) {

    if (shift_words < 0) {
        return -1;
    }

    if (shift_words == 0) {
        return 0;
    }

    if (_top + shift_words >= _total_data) {
        _big_int_expand(BI_DEFAULT_EXPAND_COUNT + shift_words);
    }
    
    for (int i = _top - 1; i >= 0; --i) {
        _data[i + shift_words] = _data[i];
    }
    _top += shift_words;

    for (int i = 0; i < shift_words; ++i) {
        _data[i] = 0;
    }
    return 0;

}

int bi::big_int::big_int_signed_sub(const bi::big_int &b) {

    int ret_val;
    bi::big_int temp_val;
    ret_val = big_int_signed_sub(b, temp_val);
    _big_int_swap(temp_val);
    return ret_val;

}

int bi::big_int::big_int_signed_sub(const bi::big_int &b, bi::big_int &res) {

    int comp_stat = big_int_compare(b);
    if (comp_stat == 0) {
        return res.big_int_set_zero();
    } else {

        // TODO: avoid expensive (might be) copy here
        bi::big_int temp_sub(b);
        temp_sub._neg = !temp_sub._neg;
        return big_int_signed_add(temp_sub, res);
    }

}

int bi::big_int::big_int_set_zero() {

    big_int_clear();
    if (_total_data > 0) {
        _top = 1;
    } else {
        return -1;
    }
    return 0;

}

int bi::big_int::big_int_unsigned_sub(const bi::big_int &b, bi::big_int &res) const {

    int max, min;
    max = _top;
    min = b._top;

    int diff = big_int_unsigned_compare(b);
    if(diff < 0) {
        throw std::length_error("First param should be larger");
    }

    res.big_int_clear();
    BI_BASE_TYPE borrow = _big_int_sub_base_type(b._data, min, res);

    if((max - min) >= res._total_data) {
        res._big_int_expand(BI_DEFAULT_EXPAND_COUNT + (max - min));
    }

    for(int i = min; i < max; i++) {
        if(compare_bi_base_type(_data[i], borrow)) {
            res._data[(res._top)++] = _data[i] - borrow;
            borrow = 0;
        } else {
            res._data[(res._top)++] = \
            static_cast<BI_BASE_TYPE>((static_cast<BI_DOUBLE_BASE_TYPE>(BI_BASE_TYPE_MAX) + 1 - borrow));
            borrow = 1;
        }
    }

    res._big_int_remove_preceding_zeroes();

    return 0;

}

int bi::big_int::big_int_unsigned_sub(const bi::big_int &b) {

    bi::big_int temp;
    big_int_unsigned_sub(b, temp);
    temp._neg = this->_neg;
    _big_int_swap(temp);
    return 0;

}

int bi::big_int::big_int_multiply(const bi::big_int &b, bi::big_int &res) {

    bi::big_int temp_bi;

    res.big_int_set_zero();

    if(big_int_is_zero() || b.big_int_is_zero()) {
        /* If any/both of the opreands is zero set result as zero and exit */
        return 0;
    }

    res._neg = _neg ^ b._neg;
    
    for (int i = 0; i < b._top; ++i) {
        temp_bi.big_int_clear();
        _big_int_unsigned_multiply_bi_base_type(b._data[i], temp_bi);
        temp_bi.big_int_left_shift_word(i);
        res.big_int_unsigned_add(temp_bi);
    }
    return 0;

}

int bi::big_int::big_int_unsigned_multiply_base_type(const BI_BASE_TYPE &b, bi::big_int &res) const {

    return _big_int_unsigned_multiply_bi_base_type(b, res);

}

int bi::big_int::big_int_clear() {

    memset(_data, 0, static_cast<size_t>(_total_data) * sizeof(BI_BASE_TYPE));
    _top            = 0;
    _neg            = false;
    return 0;
}

std::string     bi::big_int::big_int_to_string(bi::bi_base base) {

    std::string tmp_op_str, hex_str = _big_int_to_string();

    bool is_neg = false;
    if (hex_str[0] == '-') {
        is_neg = true;
        hex_str = hex_str.substr(1);
    }

    size_t n = hex_str.find_first_not_of('0');
    if (n != std::string::npos) {
        hex_str = hex_str.substr(n);
    }
    else {
        hex_str = "0";
    }

    switch (base) {
        case bi_base::BI_BIN: {
            BaseConverter hex_to_bin(hex_num_set, bin_num_set);
            tmp_op_str = hex_to_bin.Convert(hex_str);
            break;
        }
        case bi_base::BI_DEC: {
            BaseConverter hex_to_dec(hex_num_set, dec_num_set);
            tmp_op_str = hex_to_dec.Convert(hex_str);
            break;
        }
        case bi_base::BI_HEX:
            tmp_op_str = hex_str;
            break;
        default:
            throw std::invalid_argument("Invalid base");
    }   

    if (is_neg) {
        tmp_op_str.insert(0, "-");
    }

    return tmp_op_str;

}

int bi::big_int::big_int_set_negetive(bool set_unset) {

    if (big_int_is_zero()) {
        return -1;
    }
    _neg = set_unset;
    return 0;

}

bool bi::big_int::big_int_is_zero() const {

    if (_top > 1) {
        return false;
    } else if (_top == 1) {
        if (_data[0] == 0) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }

}


bool bi::big_int::big_int_is_negetive() const {

    return _neg;

}

int bi::big_int::big_int_unsigned_compare(const bi::big_int &other) const {

    return _big_int_compare_bi_base_type_n_top(other);

}

int bi::big_int::big_int_compare(const bi::big_int &other) const {

    if (_neg == other._neg) {
        if (_neg == 1) {
            return -1 * _big_int_compare_bi_base_type_n_top(other);
        } else {
            return _big_int_compare_bi_base_type_n_top(other);
        }
    } else {
        if (_neg == 1) {
            return -1;
        } else {
            return 1;
        }
    }
}

int bi::big_int::big_int_left_shift(int bits) {

    int ret_val = 0;
    int word_shifts = bits / BI_BASE_TYPE_TOTAL_BITS;
    int bit_shifts = bits % BI_BASE_TYPE_TOTAL_BITS;
    if (word_shifts > 0) {
        ret_val = big_int_left_shift_word(word_shifts);
    }
    if (bit_shifts > 0) {
        ret_val += _big_int_left_shift_below_32bits(bit_shifts);
    }
    return ret_val;

}

int bi::big_int::big_int_left_shift(int bits, bi::big_int &res) {

    int ret_val;
    big_int temp_val(*this);
    ret_val = temp_val.big_int_left_shift(bits);
    temp_val._big_int_swap(res);
    return ret_val;

}

int bi::big_int::big_int_left_shift_word(int shift_words, bi::big_int &res) {

    int ret_val;
    big_int temp_val(*this);
    ret_val = temp_val.big_int_left_shift_word(shift_words);
    temp_val._big_int_swap(res);
    return ret_val;

}

int bi::big_int::big_int_right_shift_word(int shift_words) {

    if (shift_words >= _top) {
        return big_int_set_zero();
    }

    int i = 0;
    for (; i < _top - shift_words; ++i) {
        _data[i] = _data[i + shift_words];
    }
    for (int j = 0; j < shift_words; ++j) {
        _data[i + j] = 0;
    }

    _top -= shift_words;

    return 0;

}

int bi::big_int::big_int_right_shift_word(int shift_words, bi::big_int &res) {

    int ret_val;
    big_int temp_val(*this);
    ret_val = temp_val.big_int_right_shift_word(shift_words);
    temp_val._big_int_swap(res);
    return ret_val;

}

int bi::big_int::big_int_right_shift(int bits) {

    int ret_val = 0;
    int word_shifts = bits / BI_BASE_TYPE_TOTAL_BITS;
    int bit_shifts = bits % BI_BASE_TYPE_TOTAL_BITS;
    if (word_shifts > 0) {
        ret_val = big_int_right_shift_word(word_shifts);
    }
    if (bit_shifts > 0) {
        ret_val += _big_int_right_shift_below_32bits(bit_shifts);
    }
    return ret_val;

}

int bi::big_int::big_int_right_shift(int bits, bi::big_int &res) {

    int ret_val;
    big_int temp_val(*this);
    ret_val = temp_val.big_int_right_shift(bits);
    temp_val._big_int_swap(res);
    return ret_val;

}

int bi::big_int::big_int_get_num_of_hex_chars() const {

    return _big_int_get_num_of_hex_chars();

}

int bi::big_int::big_int_push_back_hex_chars(const BI_BASE_TYPE &hex_chars) {

    return _big_int_push_back_hex_chars(hex_chars);

}

int bi::big_int::big_int_divide_once(const bi::big_int &divisor, BI_BASE_TYPE &op_quotient, bi::big_int &op_remainder) {

    return _big_int_divide_once(divisor, op_quotient, op_remainder);

}

int bi::big_int::big_int_div(const bi::big_int &divisor, bi::big_int &op_quotient, bi::big_int &op_remainder) {

    if (divisor.big_int_is_zero()) {
        // throw std::length_error("Division by zero undefined");
        return -1;
    }

    bool result_sign = big_int_is_negetive() ^ divisor.big_int_is_negetive();

    if (big_int_is_zero()) {
        /* Set remainder and quotient as zero if dividend is zero. */
        op_remainder.big_int_set_zero();
        op_quotient.big_int_set_zero();
        return 0;
    }

    int comp_res = big_int_unsigned_compare(divisor);
    switch (comp_res) {
    case -1:
        /* Divisor is greater than dividend, set quotient as zero 
        and remainder as dividend */
        op_remainder = *this;
        /* Remainder takes the sign of dividend. */
        op_remainder.big_int_set_negetive(_neg);
        op_quotient.big_int_set_zero();
        return 0;
    case 0:
        /* Both dividend and divisor are same, so set quotient as 1 and remainder as zero. */
        op_quotient.big_int_from_base_type(static_cast<BI_BASE_TYPE>(1), result_sign);
        op_remainder.big_int_set_zero();
        return 0;
    case 1:

        /* Clear quotient and remainder big ints. */
        op_quotient.big_int_set_zero();

        BI_BASE_TYPE temp_div_once_quotient, temp_new_hex_rem_lsb = 0;
        bi::big_int temp_div_once_dividend, temp_div_once_remainder; 
        int divisor_length = divisor._big_int_get_num_of_hex_chars();
        int dividend_length = _big_int_get_num_of_hex_chars();
        int ret_code = big_int_right_shift((dividend_length - divisor_length) * 4, temp_div_once_dividend);
        int divide_cntr = 0;
        
        do {
        
            /* Divide once. */
            temp_div_once_quotient = 0;
            temp_div_once_remainder.big_int_clear();
            ret_code += temp_div_once_dividend._big_int_divide_once(divisor, temp_div_once_quotient, temp_div_once_remainder);
            ret_code += op_quotient._big_int_push_back_hex_chars(temp_div_once_quotient);

            if ((dividend_length - divisor_length - divide_cntr - 1) >= 0) {
                ret_code += _big_int_get_hex_char_from_lsb(dividend_length - divisor_length - divide_cntr - 1, temp_new_hex_rem_lsb);
            } 
            
            temp_div_once_dividend = temp_div_once_remainder;
            ret_code += temp_div_once_dividend.big_int_push_back_hex_chars(temp_new_hex_rem_lsb);
            ++divide_cntr;
        
        } while ((dividend_length - divisor_length + 1 - divide_cntr) > 0 && ret_code == 0);
        
        op_remainder = temp_div_once_remainder;
        op_quotient.big_int_set_negetive(result_sign);

        /* Remainder takes the sign of dividend. */
        op_remainder.big_int_set_negetive(_neg);

        return ret_code;

    }

    /* Should't reach here. */
    return -1;

}

int bi::big_int::big_int_power_base_type(const BI_BASE_TYPE &exponent, big_int &result) {

    int ret_val = 0;

    if (big_int_is_zero()) {
        if (exponent == 0) {
            /* pow(0, 0) = 1 */
            return result.big_int_from_base_type(1, false);
        } else {
            /* pow(0, anything except 0) = 0 */
            return result.big_int_set_zero();
        }
    }
    
    ret_val += result.big_int_from_base_type(1, false);

    big_int temp_val;
    for (BI_BASE_TYPE i = 0; i < exponent; ++i) {
        ret_val += big_int_multiply(result, temp_val);
        result = temp_val;
    }

    return ret_val;

}

int bi::big_int::big_int_modulus(const big_int &modulus, big_int &result) {

    big_int temp_quo, temp_rem;
    int ret_val = 0;

    ret_val += big_int_div(modulus, temp_quo, temp_rem);
    if (modulus.big_int_is_negetive() == false) {
        if (temp_rem.big_int_is_negetive() == true) {
            ret_val += modulus.big_int_unsigned_sub(temp_rem, result);
        } else {
            result = temp_rem;
        }
    } else {
        if ((temp_rem.big_int_is_negetive() == false) && (temp_rem.big_int_is_zero() == false)) {
            ret_val += modulus.big_int_unsigned_sub(temp_rem, result);   
        } else {
            result = temp_rem;
        }
        if (result.big_int_is_zero() == false) {
            ret_val += result.big_int_set_negetive(true);
        }
    }
    return ret_val;

}

bool bi::big_int::big_int_is_even() const {

    if (_top > 0) {
        if (_data[0] % 2 == 0) {
            return true;
        } else {
            return false;
        }
    } else {
        /* TODO: catch error. */
        return true;
    }

}


/*

    Fast modular exponentiation algorithm
    -------------------------------------
    
    [refer](https://stackoverflow.com/questions/40578553/fast-modular-exponentiation-help-me-find-the-mistake)
    [refer](http://homepages.math.uic.edu/~leon/cs-mcs401-s08/handouts/fastexp.pdf)

    Algorithm - python:
    -------------------

        def pow_h(base, power, modulus):
            a = 1
            while power:
                power, d = power // 2, power % 2
                if d:
                    a = a * base % modulus
                base = base * base % modulus
            return a

    Example:    8 ^ 5 mod 7

    Result is initialized to 1
    prev_base = 7, initial base during first iteration
    n_base => next base

    +----------------------+----------------------+----------------------+----------------------+
    | Itertion             | Exponent bit         | Calculation          | Result               |
    +----------------------+----------------------+----------------------+----------------------+
    | 1                    | 1                    | prev_base = 7        | 1 * 8 mod 7 = 1      | 
    |                      |                      | n_base = 8 mod 7 = 1 |                      |
    +----------------------+----------------------+----------------------+----------------------+
    | 2                    | 0                    | n_base = 8 ^ 2 mod 7 | No result calc. if   |
    |                      |                      | = 1 * 1 mod 7 = 1    | exponent bit is zero |
    +----------------------+----------------------+----------------------+----------------------+
    | 3                    | 1                    | n_base = 8 ^ 4 mod 7 | 1 * 1 mod 7 = 1      |
    |                      |                      | = 8 ^ 2 * 8 ^ 2 mod 7|                      |
    |                      |                      | = 1 * 1 mod 7 = 1    |                      |
    +----------------------+----------------------+----------------------+----------------------+

*/

int bi::big_int::big_int_fast_modular_exponentiation(const big_int &exponent, const big_int &modulus, big_int &result) {

    int ret_val = 0;

    big_int bi_1;
    bi_1.big_int_from_base_type(1, false);
    /* Compare modulus with one. */
    int comp_res = modulus.big_int_unsigned_compare(bi_1);
    /* Compare exponent with one. */
    int exp_comp_res = exponent.big_int_unsigned_compare(bi_1);

    if (exp_comp_res == 0) {
        if (exponent.big_int_is_negetive() == false) {
            /* Compare exponent with 1, return early with mod of base. */
            return (*this).big_int_modulus(modulus, result);
        } else {
            /* Compare exponent with -1, return early with inverse of base. */
            try {
                ret_val += (*this).big_int_modular_inverse_extended_euclidean_algorithm(modulus, result);
            } catch (...) {
                throw;
            }
            return ret_val;
        }
    }
    
    if (modulus.big_int_is_zero() == true) {
        /* If modulus is zero throw error. */
        throw std::range_error("Modulus cannot be zero.");
    } else if (comp_res == 0) {
        /* Modulus is +/-1 */
        return result.big_int_set_zero();
    } else if ((*this).big_int_is_zero() == true) {
        /* Handle base = 0 */
        if (exponent.big_int_is_negetive() == false && exponent.big_int_is_zero() == false) {
            return result.big_int_set_zero();
        } else if (exponent.big_int_is_negetive() == true) {
            throw std::range_error("Given base (0) doesn't have inverse. ");
        } else if (exponent.big_int_is_zero() == true) {
            if (modulus.big_int_is_negetive() == false) {
                result = bi_1;
            } else {
                ret_val += modulus.big_int_unsigned_sub(bi_1, result);  
                ret_val += result.big_int_set_negetive(true);
            }
            return ret_val;
        }
    }
    
    if (exponent.big_int_is_negetive() == false) {
        /* +ve exponent, follow usual algorithm. */
        ret_val += (*this)._big_int_fast_modular_exponentiation(exponent, modulus, result);
    } else {
        /* -ve exponent,  find inverse of base first. */
        big_int temp_inverse;
        try {
            ret_val += (*this).big_int_modular_inverse_extended_euclidean_algorithm(modulus, temp_inverse);
        } catch (...) {
            throw;
        }
        big_int us_exp(exponent);
        us_exp.big_int_set_negetive(false);
        /* Do modular exponentiation on inverse of the original base to get the final result. */
        ret_val += temp_inverse.big_int_fast_modular_exponentiation(us_exp, modulus, result);
    }
    return ret_val;

}

/*

    GCD - Euclidean algorithm
    -------------------------

    [refer](https://en.wikipedia.org/wiki/Euclidean_algorithm)

    For illustration, the Euclidean algorithm can be used to find the greatest common divisor of a = 1071 and b = 462. 
    To begin, multiples of 462 are subtracted from 1071 until the remainder is less than 462. Two such multiples can be subtracted (q0 = 2), 
    leaving a remainder of 147:

        1071 = 2 × 462 + 147.
    
    Then multiples of 147 are subtracted from 462 until the remainder is less than 147. Three multiples can be subtracted (q1 = 3), leaving a remainder of 21:

        462 = 3 × 147 + 21.
    
    Then multiples of 21 are subtracted from 147 until the remainder is less than 21. Seven multiples can be subtracted (q2 = 7), leaving no remainder:

        147 = 7 × 21 + 0.
    
    Since the last remainder is zero, the algorithm ends with 21 as the greatest common divisor of 1071 and 462. 

*/

int bi::big_int::big_int_gcd_euclidean_algorithm(const big_int &b, big_int &op_gcd) {

    if (((*this).big_int_is_zero() & b.big_int_is_zero()) == true) {
        /* If both numbers are zero gcd is zero. */
        return op_gcd.big_int_set_zero();
    } else if ((*this).big_int_is_zero()) {
        /* If one of the number is zero and other is non zero then gcd is the non
        zero number. */
        op_gcd = b;
        return  op_gcd.big_int_set_negetive(false);
    } else if (b.big_int_is_zero()) {
        /* If one of the number is zero and other is non zero then gcd is the non
        zero number. */
        op_gcd = (*this);
        return op_gcd.big_int_set_negetive(false);    
    }

    /* Temp working variables. */
    big_int temp_greater, temp_lower, temp_quo, temp_rem, prev_rem;

    /* Compare and assign greater and lower big int temp variables. */
    int ret_code = 0, comp_stat = (*this).big_int_unsigned_compare(b);
    if (comp_stat == 0) {
        /* If both numbers are equal then gcd is equal to the +ve number. */
        op_gcd = (*this);
        return op_gcd.big_int_set_negetive(false);
    } else if (comp_stat == 1) {
        temp_greater = (*this);
        temp_lower = b;
    } else {
        temp_greater = b;
        temp_lower = (*this);
    }

    /* Set the numbers as positve as the gcd is +ve. */
    temp_greater.big_int_set_negetive(false); temp_lower.big_int_set_negetive(false);

    /* Initialse the temp_rem and thus prev_rem with the lower number so that,
    the gcd will be the lower number when both numbers are multiples. */
    temp_rem = temp_lower;
    do {
        /* GCD Euclidean algorithm (refer func. docs.) */
        prev_rem = temp_rem;
        ret_code += temp_greater.big_int_div(temp_lower, temp_quo, temp_rem);
        temp_greater = temp_lower;
        temp_lower = temp_rem;
    } while(temp_rem.big_int_is_zero() == false);

    /* Assign previous remainder as the gcd. */
    op_gcd = prev_rem;
    return ret_code;

}

/*

    The Extended Euclidean Algorithm
    --------------------------------

[refer](http://www-math.ucdenver.edu/~wcherowi/courses/m5410/exeucalg.html)

We will number the steps of the Euclidean algorithm starting with step 0. 
The quotient obtained at step i will be denoted by qi. As we carry out each 
step of the Euclidean algorithm, we will also calculate an auxillary number, pi. 
For the first two steps, the value of this number is given: p0 = 0 and p1 = 1. 
For the remainder of the steps, we recursively calculate pi = [pi-2 - (pi-1 * qi-2)] (mod n). 
Continue this calculation for one step beyond the last step of the Euclidean algorithm.

The algorithm starts by "dividing" n by x. If the last non-zero remainder occurs at step k, 
then if this remainder is 1, x has an inverse and it is pk+2. (If the remainder is not 1, then x does not have an inverse.) 
Here is an example:

Find the inverse of 15 mod 26.

    Step 0:	26 = 1(15) + 11	        p0 = 0
    Step 1:	15 = 1(11) + 4	        p1 = 1
    Step 2:	11 = 2(4) + 3	        p2 = 0 - 1( 1) mod 26   = 25
    Step 3:	4  = 1(3) + 1	        p3 = 1 - 25( 1) mod 26  = -24 mod 26 = 2
    Step 4:	3  = 3(1) + 0	        p4 = 25 - 2( 2) mod 26  = 21
                                    p5 = 2 - 21( 1) mod 26  = -19 mod 26 = 7

*/

int bi::big_int::big_int_modular_inverse_extended_euclidean_algorithm(const big_int &ip_modulus, big_int &inverse) {

    big_int bi_1;
    bi_1.big_int_from_base_type(1, false);

    /* Extended euclidean algorithm (EEA) working variables */
    big_int pk_0, pk_1, pk_temp_1, pk_temp_2;
    int step_cntr = 0;
    /* Temp working variables. */
    big_int temp_greater, temp_lower, temp_quo, temp_rem, prev_rem, prev_quo[2];

    /* Temporary working copies. */
    big_int ip_num(*this), modulus(ip_modulus);
    int ret_code = 0;

    /* Do calculation as if they are +ve numbers. */
    ip_num.big_int_set_negetive(false);
    modulus.big_int_set_negetive(false);
    int comp_stat = ip_num.big_int_compare(modulus);

    if (modulus.big_int_is_zero() == true) {
        /* No inverse if modulus is equal to zero. */
        throw std::range_error("The number is not invertible for the given modulus");
    } else if (ip_modulus.big_int_unsigned_compare(bi_1) == 0) {
        /* If modulus is 1 or -1 then inverse is zero. */
        return inverse.big_int_set_zero();
    } else if ((*this).big_int_unsigned_compare(bi_1) == 0) {
        /* If the number is 1 set the inverse 1, follow the sign based inverse change. */
        pk_1.big_int_from_base_type(1, false);
        goto change_inverse_based_on_arg_sign;
    }

    if (comp_stat == 0 || ip_num.big_int_is_zero() == true) {
        /* No inverse if number is equal to zero. */
        throw std::range_error("The number is not invertible for the given modulus");
    } else if (comp_stat > 0) {
        /* If greater then reduce. */
        big_int temp_res;
        ret_code += ip_num.big_int_modulus(modulus, temp_res);
        ip_num = temp_res;
    }

    /* Init the variables to 0 and 1. */
    pk_1.big_int_from_base_type(1, false);

    temp_greater = modulus;
    temp_lower = ip_num;
    temp_rem = temp_lower;

    /* Extended euclidean algorithm */
    do {
        ++step_cntr;
        prev_rem = temp_rem;
        prev_quo[0] = prev_quo[1];
        prev_quo[1] = temp_quo;
        ret_code += temp_greater.big_int_div(temp_lower, temp_quo, temp_rem);
        temp_greater = temp_lower;
        temp_lower = temp_rem;
        if (step_cntr > 2) {
            /* pi = [pi-2 - (pi-1 * qi-2)] (mod n) */
            ret_code += pk_1.big_int_multiply(prev_quo[0], pk_temp_1);
            ret_code += pk_0.big_int_signed_sub(pk_temp_1, pk_temp_2);
            pk_0 = pk_1;
            ret_code += pk_temp_2.big_int_modulus(modulus, pk_1);
        }
    } while(temp_rem.big_int_is_zero() == false);

    if (prev_rem.big_int_compare(bi_1) != 0) {
        /* Throw if the GCD of the args is not 1, which means, the
        numbers are not co - primes. */ 
        throw std::range_error("The number is not invertible for the given modulus");
    }

    /* Final iteration. */
    ret_code += pk_1.big_int_multiply(prev_quo[1], pk_temp_1);
    ret_code += pk_0.big_int_signed_sub(pk_temp_1, pk_temp_2);
    ret_code += pk_temp_2.big_int_modulus(modulus, pk_1);

change_inverse_based_on_arg_sign:
    /* Use the argument signs to change final pk_1. 
    Sign convention is similar to big_int_modulus() */
    if (ip_modulus.big_int_is_negetive() == false) {
        if ((*this).big_int_is_negetive() == true) {
            ret_code += modulus.big_int_unsigned_sub(pk_1, inverse);
        } else {
            inverse = pk_1;
        }
    } else {
        if (((*this).big_int_is_negetive() == false) && ((*this).big_int_is_zero() == false)) {
            ret_code += modulus.big_int_unsigned_sub(pk_1, inverse);   
        } else {
            inverse = pk_1;
        }
        ret_code += inverse.big_int_set_negetive(true);
    }

    return ret_code;

}


int bi::big_int::big_int_get_random_unsigned(int bits) {

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<BI_BASE_TYPE> rand_dist(0, 0xFFFFFFFF);

    return _big_int_generate_random_unsigned(bits, rng, rand_dist);

}

int bi::big_int::big_int_fast_divide_by_power_of_two(int power, big_int &remainder, big_int &quotient) const {

    /* Take the remainder. */
    int ret_val = 0;
    int word_cnt = power / BI_BASE_TYPE_TOTAL_BITS;
    int bit_cont = power % BI_BASE_TYPE_TOTAL_BITS;

    if (word_cnt >= _top) {
        ret_val += quotient.big_int_set_zero();
        remainder = *this;
        return ret_val;
    } 

    remainder.big_int_clear();

    int i = 0; 
    for (; i < word_cnt; ++i) {
        remainder._data[(remainder._top)++] = (*this)._data[i];
    }

    BI_DOUBLE_BASE_TYPE temp_dbase = (*this)._data[i];
    temp_dbase <<= (BI_BASE_TYPE_TOTAL_BITS - bit_cont);
    temp_dbase &= BI_BASE_TYPE_MAX;
    remainder._data[(remainder._top)++] = static_cast<BI_BASE_TYPE>(temp_dbase >> (BI_BASE_TYPE_TOTAL_BITS - bit_cont)); 
    remainder.big_int_set_negetive((*this)._neg);

    quotient = (*this);
    return quotient.big_int_right_shift(power); 

}

int bi::big_int::big_int_fast_multiply_by_power_of_two(int power, big_int &result) const {

    result = (*this);
    return result.big_int_left_shift(power);

}

int bi::big_int::big_int_get_num_of_bits() const {

    return 4 * big_int_get_num_of_hex_chars();

}

int bi::big_int::big_int_get_random_unsigned_between(const big_int &low, const big_int &high) {


    std::random_device rd;                              // only used once to initialise (seed) engine
    std::mt19937 rng(rd());                             // random-number engine used (Mersenne-Twister in this case)
    std::uniform_int_distribution<BI_BASE_TYPE> uni_dist(0, 0xFFFFFFFF);
    std::uniform_int_distribution<int> get_rand_bits(low.big_int_get_num_of_bits(), high.big_int_get_num_of_bits());    

    return _big_int_get_random_unsigned_between(rng, uni_dist, get_rand_bits, low, high);

}

int bi::big_int::big_int_get_random_unsigned_prime_rabin_miller(int bits, int reqd_rabin_miller_iterations) {

    int ret_val = 0;

    std::random_device rd;
    std::mt19937 rng(rd()); 
    std::uniform_int_distribution<BI_BASE_TYPE> uni_dist(0, 0xFFFFFFFF);
    
    while (ret_val == 0) {

        big_int candidate_num, bi_1, bi_2, candidate_num_sub_1, candidate_num_sub_1_copy, prev_candidate_num_sub_1;
        ret_val += candidate_num._big_int_generate_random_probable_prime(bits, rng, uni_dist, -1); /* -1 -> Use all prime numbers in the array. */ 
        ret_val += bi_1.big_int_from_base_type(1, false);
        ret_val += bi_2.big_int_from_base_type(2, false);
        std::uniform_int_distribution<int> uni_dist_rand_bits(bi_2.big_int_get_num_of_bits(), candidate_num.big_int_get_num_of_bits());

        int max_div_by_two = 0;
        BI_BASE_TYPE div_2_rem;
        ret_val += candidate_num.big_int_unsigned_sub(bi_1, candidate_num_sub_1);
        candidate_num_sub_1_copy = candidate_num_sub_1;

        prev_candidate_num_sub_1 = candidate_num_sub_1;
        ret_val += candidate_num_sub_1._big_int_fast_divide_by_two(div_2_rem);
        while (div_2_rem == 0) {
            ++max_div_by_two;
            prev_candidate_num_sub_1 = candidate_num_sub_1;
            ret_val += candidate_num_sub_1._big_int_fast_divide_by_two(div_2_rem);
        }

        //std::cout<<"d: "<<max_div_by_two<<" num: "<<prev_candidate_num_sub_1.big_int_to_string(bi_base::BI_HEX)<<"\n";
        //break;
        int i = 0;
        for (; i < reqd_rabin_miller_iterations; ++i) {
            big_int this_round_random_bi;
            ret_val += this_round_random_bi._big_int_get_random_unsigned_between(rng, uni_dist, uni_dist_rand_bits, bi_2, candidate_num);

            big_int mod_exp_res;
            bool composite_test = true;
            ret_val += this_round_random_bi.big_int_fast_modular_exponentiation(prev_candidate_num_sub_1, candidate_num, mod_exp_res);
            if (mod_exp_res.big_int_unsigned_compare(bi_1) == 0) {
                composite_test = false;
            }
            else {
                for (int j = 0; j < max_div_by_two; ++j) {
                    big_int temp_exp, fast_mul_bi_1(bi_1);
                    ret_val += fast_mul_bi_1.big_int_left_shift(j);
                    ret_val += prev_candidate_num_sub_1.big_int_multiply(fast_mul_bi_1, temp_exp);
                    ret_val += this_round_random_bi.big_int_fast_modular_exponentiation(temp_exp, candidate_num, mod_exp_res);
                    if (mod_exp_res.big_int_unsigned_compare(candidate_num_sub_1_copy) == 0) {
                        composite_test = false;
                        break;
                    }
                }
            }

            if (composite_test == true) {
                break;
            }
        }

        if (i == reqd_rabin_miller_iterations) {
            (*this) = candidate_num;
            break;
        }

    }

    return ret_val;
    
}

/*
reqd_rabin_miller_iterations -> use 20?
no_of_threads -> number of threads used to run the function, -ve or 0 thread count causes maximum (std::thread::hardware_concurrency();)
                 threads to be used, more than std::thread::hardware_concurrency(); will cause max threads to be std::thread::hardware_concurrency();

*/
int bi::big_int::big_int_get_random_unsigned_prime_rabin_miller_threaded(int bits, int reqd_rabin_miller_iterations, int no_of_threads) {

    std::mutex          final_op_mutex;
    big_int             final_op;
    std::atomic<bool>   stop_thread{false};

    auto rabin_miller_lambda = [&] {

        int ret_val = 0;

        std::random_device rd;
        std::mt19937 rng(rd()); 
        std::uniform_int_distribution<BI_BASE_TYPE> uni_dist(0, 0xFFFFFFFF);
        
        while (ret_val == 0 && !stop_thread) {

            big_int candidate_num, bi_1, bi_2, candidate_num_sub_1, candidate_num_sub_1_copy, prev_candidate_num_sub_1;
            ret_val += candidate_num._big_int_generate_random_probable_prime(bits, rng, uni_dist, -1); /* -1 -> Use all prime numbers in the array. */ 
            ret_val += bi_1.big_int_from_base_type(1, false);
            ret_val += bi_2.big_int_from_base_type(2, false);
            std::uniform_int_distribution<int> uni_dist_rand_bits(bi_2.big_int_get_num_of_bits(), candidate_num.big_int_get_num_of_bits());

            int max_div_by_two = 0;
            BI_BASE_TYPE div_2_rem;
            ret_val += candidate_num.big_int_unsigned_sub(bi_1, candidate_num_sub_1);
            candidate_num_sub_1_copy = candidate_num_sub_1;

            prev_candidate_num_sub_1 = candidate_num_sub_1;
            ret_val += candidate_num_sub_1._big_int_fast_divide_by_two(div_2_rem);
            while (div_2_rem == 0 && !stop_thread) {
                ++max_div_by_two;
                prev_candidate_num_sub_1 = candidate_num_sub_1;
                ret_val += candidate_num_sub_1._big_int_fast_divide_by_two(div_2_rem);
            }

            //std::cout<<"d: "<<max_div_by_two<<" num: "<<prev_candidate_num_sub_1.big_int_to_string(bi_base::BI_HEX)<<"\n";
            //break;
            int i = 0;
            for (; i < reqd_rabin_miller_iterations && !stop_thread; ++i) {
                big_int this_round_random_bi;
                ret_val += this_round_random_bi._big_int_get_random_unsigned_between(rng, uni_dist, uni_dist_rand_bits, bi_2, candidate_num);

                big_int mod_exp_res;
                bool composite_test = true;
                ret_val += this_round_random_bi.big_int_fast_modular_exponentiation(prev_candidate_num_sub_1, candidate_num, mod_exp_res);
                if (mod_exp_res.big_int_unsigned_compare(bi_1) == 0) {
                    composite_test = false;
                }
                else {
                    for (int j = 0; j < max_div_by_two && !stop_thread; ++j) {
                        big_int temp_exp, fast_mul_bi_1(bi_1);
                        ret_val += fast_mul_bi_1.big_int_left_shift(j);
                        ret_val += prev_candidate_num_sub_1.big_int_multiply(fast_mul_bi_1, temp_exp);
                        ret_val += this_round_random_bi.big_int_fast_modular_exponentiation(temp_exp, candidate_num, mod_exp_res);
                        if (mod_exp_res.big_int_unsigned_compare(candidate_num_sub_1_copy) == 0) {
                            composite_test = false;
                            break;
                        }
                    }
                }

                if (composite_test == true) {
                    break;
                }
            }

            if (i == reqd_rabin_miller_iterations) {
                std::unique_lock<std::mutex> op_value_lock(final_op_mutex);
                final_op = candidate_num;
                op_value_lock.unlock();
                stop_thread = true;
                break;
            }

        }

        return ret_val;

    };

    size_t total_thread_count;
    if (no_of_threads <= 0) {
        total_thread_count = std::thread::hardware_concurrency();
    } else if (static_cast<size_t>(no_of_threads) > std::thread::hardware_concurrency()) {
        total_thread_count = std::thread::hardware_concurrency();
    } else {
        total_thread_count = static_cast<size_t>(no_of_threads);
    }

    std::vector<std::thread> rabin_miller_threads;
    rabin_miller_threads.reserve(total_thread_count);
    for(auto i = 0u; i < total_thread_count; ++i) {
        rabin_miller_threads.emplace_back(rabin_miller_lambda);
    }

    for(auto &t : rabin_miller_threads) {
        t.join();
    }

    (*this) = final_op;
    return 0;

}