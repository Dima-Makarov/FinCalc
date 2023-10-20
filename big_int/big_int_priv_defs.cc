/** 
 *  @file   big_int_priv_defs.cc
 *  @brief  Big int private member definitions
 *
 *  @author         Tony Josi   https://tonyjosi97.github.io/profile/
 *  @copyright      Copyright (C) 2021 Tony Josi
 *  @bug            No known bugs.
 */

#include <algorithm>
#include <stdexcept>
#include <memory>

#include "big_int.hpp"
#include "big_int_lib_log.hpp"
#include "big_int_inline_defs.hpp"

namespace {
    const BI_BASE_TYPE first_primes_list[] = {   
                                                2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,
                                                61,67,71,73,79,83,89,97,101,103,107,109,113,127,
                                                131,137,139,149,151,157,163,167,173,179,181,191,
                                                193,197,199,211,223,227,229,233,239,241,251,257,
                                                263,269,271,277,281,283,293,307,311,313,317,331,
                                                337,347,349,353,359,367,373,379,383,389,397,401
                                                // 409,419,421,431,433,439,443,449,457,461,463,467,
                                                // 479,487,491,499,503,509,521,523,541,547,557,563,
                                                // 569,571,577,587,593,599,601,607,613,617,619,631,
                                                // 641,643,647,653,659,661,673,677,683,691,701,709,
                                                // 719,727,733,739,743,751,757,761,769,773,787,797,
                                                // 809,811,821,823,827,829,839,853,857,859,863,877,
                                                // 881,883,887,907,911,919,929,937,941,947,953,967,
                                                // 971,977,983,991,997
                                            };
}

void bi::big_int::_big_int_swap(bi::big_int &src) {

    using std::swap;

    swap(_data,         src._data);
    swap(_total_data,   src._total_data);
    swap(_top,          src._top);
    swap(_neg,          src._neg);

}

int bi::big_int::_big_int_expand(int req) {

    if (req > 0) {
        BI_BASE_TYPE *temp_buff = new BI_BASE_TYPE[_total_data + req];
        if(!temp_buff) {
            _BI_LOG(1, "_big_int_expand failed");
            throw std::length_error("Couldnt find enough memory");
        } else {
            std::copy_n(_data, _total_data, temp_buff);
            delete[] _data;
            _data = temp_buff;
            _total_data += req;
        }
        _BI_LOG(2, "_big_int_expand expanded to total: %d items", _total_data);
        return 0;
    } else {
        _BI_LOG(1, "_big_int_expand fail negetive expand value");
        return -1;
    }

}

int bi::big_int::_big_int_from_string(const std::string &str_num) {

    size_t str_size = str_num.length(), num_decor_cnt = 0;
    size_t is_neg = 0;
    if (str_size > 0 && str_num[0] == '-') {
        is_neg = 1;
        num_decor_cnt += 1;
    }
    if((str_size > (1 + is_neg)) && str_num[is_neg] == '0' && \
    (str_num[is_neg + 1] == 'x' || str_num[is_neg + 1] == 'X')) {
        num_decor_cnt += 2;
    }

    str_size -= num_decor_cnt;

    /* Clear any previous data. */
    big_int_clear();

    size_t extr_space_reqd = ((str_size % BI_HEX_STR_TO_DATA_SIZE == 0) ? \
    0 : (BI_HEX_STR_TO_DATA_SIZE - (str_size % BI_HEX_STR_TO_DATA_SIZE)));
    
    size_t base_t_aligned_size = str_size + extr_space_reqd;
    std::unique_ptr<char []> temp_str(new char[base_t_aligned_size]);
    memset(temp_str.get(), '0', extr_space_reqd);
    memcpy(temp_str.get() + extr_space_reqd, str_num.c_str() + num_decor_cnt, str_size);

    int str_cur_indx = static_cast<int>(base_t_aligned_size - BI_HEX_STR_TO_DATA_SIZE);

    if(static_cast<int>((base_t_aligned_size / BI_HEX_STR_TO_DATA_SIZE) + 1) >= _total_data) {
        _big_int_expand(BI_DEFAULT_EXPAND_COUNT + static_cast<int>((base_t_aligned_size / BI_HEX_STR_TO_DATA_SIZE) + 1));
    }
    
    for(; str_cur_indx >= 0; str_cur_indx -= static_cast<int>(BI_HEX_STR_TO_DATA_SIZE)) {
        if(sscanf(&(temp_str.get()[str_cur_indx]), BI_SSCANF_FORMAT_HEX, &_data[_top++]) == EOF) {
            big_int_clear();
            return -1;
        }
    }

    if(big_int_is_zero() == true && is_neg) {
        return -1;
    }
    _neg = static_cast<bool>(is_neg);

    /* Remove extra zeroes in the MSB if the i/p string had them, except the last zero (to denote zero big integer).*/
    _big_int_remove_preceding_zeroes();

    return 0;

}

std::string bi::big_int::_big_int_to_string() {

    size_t chars_per_data = BI_SPRINF_FORMAT_HEX_CHARS;

    if (_top <= 0) {
        throw std::length_error("Invalid number: zero length");
    }

    size_t is_neg = 0;
    if (big_int_is_negetive()) {
        is_neg = 1;
    }

    std::unique_ptr<char []> char_temp_buff(new char[static_cast<size_t>(_top) * chars_per_data + 1 + is_neg]);
    memset(char_temp_buff.get(), '\0', static_cast<size_t>(_top) * chars_per_data + 1 + is_neg);
    if (is_neg) {
        char_temp_buff.get()[0] = '-';
    }

    for(int i = _top - 1; i >= 0; --i) {

        sprintf(char_temp_buff.get() + is_neg + ((static_cast<size_t>(_top) - 1) - static_cast<size_t>(i)) * \
        chars_per_data, BI_SPRINF_FORMAT_HEX, _data[i]);
        _BI_LOG(3, BI_SPRINF_FORMAT_HEX_LOG, _data[i]);
    
    }

    std::string op_string(char_temp_buff.get());
    return op_string;

}

BI_BASE_TYPE bi::big_int::_big_int_sub_base_type(BI_BASE_TYPE *data_ptr, int min, bi::big_int &res_ptr) const {

    BI_BASE_TYPE borrow = 0;
    BI_DOUBLE_BASE_TYPE diff, temp1;
    if (res_ptr._total_data <= min) {
        res_ptr._big_int_expand(BI_DEFAULT_EXPAND_COUNT + min);
    }
    for(int i = 0; i < min; ++i) {
        if(compare_bi_base_type(_data[i], data_ptr[i])) {
            diff = _data[i] - data_ptr[i] - borrow;
            borrow = 0;
        } else {
            temp1 = static_cast<BI_DOUBLE_BASE_TYPE>(_data[i]) + BI_BASE_TYPE_MAX + 1 - borrow;
            diff = temp1 - data_ptr[i];
            borrow = 1;
        }
        res_ptr._data[(res_ptr._top)++] = static_cast<BI_BASE_TYPE>(diff);
    }
    return borrow;
}

int bi::big_int::_big_int_compare_bi_base_type_n_top(const bi::big_int &other) const {

    if (_top == other._top) {
        int i = _top - 1;
        for(; i >= 0; i--) {
            if (_data[i] == other._data[i]) {
                continue;
            } else if (_data[i] > other._data[i]) {
                return 1;
            } else {
                return -1;
            }
        }
        return 0;
    } else if (_top > other._top) {
        return 1;
    } else {
        return -1;
    }

}

int bi::big_int::_big_int_unsigned_multiply_bi_base_type(BI_BASE_TYPE b, bi::big_int &res_ptr) const {

    BI_DOUBLE_BASE_TYPE interim_res;
    BI_BASE_TYPE        carry = 0;

    res_ptr.big_int_clear();

    if (_top >= res_ptr._total_data) {
        res_ptr._big_int_expand(BI_DEFAULT_EXPAND_COUNT + _top);
    }

    for(int i = 0; i < _top; ++i) {
        interim_res = static_cast<BI_DOUBLE_BASE_TYPE>(_data[i]) * b + carry;
        res_ptr._data[(res_ptr._top)++] = interim_res & BI_BASE_TYPE_MAX;
        carry = static_cast<BI_BASE_TYPE>(interim_res >> BI_BASE_TYPE_TOTAL_BITS);
    }

    if (carry) {
        if (res_ptr._top >= res_ptr._total_data) {
            res_ptr._big_int_expand(BI_DEFAULT_EXPAND_COUNT);
        }
        res_ptr._data[(res_ptr._top)++] = carry;
    }

    return 0;

}

int bi::big_int::_big_int_left_shift_below_32bits(int bits) {

    if (bits > 32) {
        return -1;
    }

    BI_DOUBLE_BASE_TYPE interim_res;
    BI_BASE_TYPE        carry = 0;

    if (_top >= _total_data) {
        _big_int_expand(BI_DEFAULT_EXPAND_COUNT + _top);
    }

    for (int i = 0; i < _top; ++i) {
        interim_res = (static_cast<BI_DOUBLE_BASE_TYPE>(_data[i]) << bits) + carry;
        carry = static_cast<BI_BASE_TYPE>(interim_res >> BI_BASE_TYPE_TOTAL_BITS);
        _data[i] = static_cast<BI_BASE_TYPE>(interim_res & BI_BASE_TYPE_MAX);
    }

    if (carry) {
        _data[_top++] = carry;
    }

    return 0;

}

int bi::big_int::_big_int_right_shift_below_32bits(int bits) {

    if (bits > 32) {
        return -1;
    }

    BI_DOUBLE_BASE_TYPE interim_res_1, interim_res_2;
    BI_BASE_TYPE        carry = 0;

    for (int i = _top - 1; i >= 0; --i) {
        interim_res_1 = static_cast<BI_DOUBLE_BASE_TYPE>(_data[i]) << BI_BASE_TYPE_TOTAL_BITS;
        interim_res_2 = (interim_res_1 >> bits);
        _data[i] = static_cast<BI_BASE_TYPE>((interim_res_2 & BI_DOUBLE_BASE_TYPE_FIRST_HALF_MASK) >> BI_BASE_TYPE_TOTAL_BITS) + carry;
        carry = interim_res_2 & BI_BASE_TYPE_MAX;
    }

    /* No need to save carry as its a right shift and those bits are discarded. */

    return _big_int_remove_preceding_zeroes();

}

int bi::big_int::_big_int_remove_preceding_zeroes() {

    int i = _top - 1;
    for(; i > 0; i--) {
        if(_data[i] == 0) {
            _top--;
        } else {
            return 0;
        }
    }

    if(_data[0] == 0) {
        /* The big int is zero, set the sign to positve just in case. */
        _neg = false;
    }

    return 0;

}

int bi::big_int::_big_int_get_num_of_hex_chars() const {

    int ret_val = 0;
    if (_top > 1) {
        ret_val += (_top - 1) * 8;
    }

    if (_top >= 1) {
        BI_BASE_TYPE temp_val = _data[_top - 1];
        
        do {
            ret_val += 1;
        } while (temp_val >>= 4);
    }

    return ret_val;

}

int bi::big_int::_big_int_divide_once(const big_int &divisor, BI_BASE_TYPE &op_quotient, big_int &op_remainder) {

    if (divisor.big_int_is_zero()) {
        // throw std::length_error("Division by zero undefined");
        return -1;
    }

    if (_big_int_get_num_of_hex_chars() - divisor._big_int_get_num_of_hex_chars() > 1) {
        /* Return error as this api will only divide once. */
        return -1;
    }

    if (big_int_is_zero()) {
        /* Set remainder and quotient as zero if dividend is zero. */
        op_remainder.big_int_set_zero();
        op_quotient = 0;
        return 0;
    }

    int comp_res = big_int_unsigned_compare(divisor);
    switch (comp_res) {
    case -1:
        /* Divisor is greater than dividend, set quotient as zero 
        and remainder as dividend */
        op_remainder = *this;
        op_quotient = 0;
        return 0;
    case 0:
        /* Both dividend and divisor are same, so set quotient as 1 and remainder as zero. */
        op_quotient = 1;
        op_remainder.big_int_set_zero();
        return 0;
    }

    big_int temp_val, temp_val_2;

    /* Start from 2 as the cases 0 & 1 are covered already in the above lines. */
    BI_BASE_TYPE i = 2;
    for (; i <= 0x10; ++i) {
        divisor.big_int_unsigned_multiply_base_type(i, temp_val);
        comp_res = big_int_unsigned_compare(temp_val);
        switch (comp_res) {
        case -1:
            op_quotient = i - 1;
            divisor.big_int_unsigned_multiply_base_type(i - 1, temp_val_2);
            big_int_unsigned_sub(temp_val_2, op_remainder);
            return 0;
        case 0:
            op_quotient = i;
            op_remainder.big_int_set_zero();
            return 0;
        }
    }

    /* Function shouldn't reach here. ERROR. */
    return -1;

}

int bi::big_int::_big_int_push_back_hex_chars(const BI_BASE_TYPE &hex_char) {

    if (hex_char > 0xF) {
        return -1;
    }

    if (hex_char == 0 && big_int_is_zero() == true) {
        /* MSB is zero, do nothing */
        return 0;
    }

    int ret_val;
    ret_val = big_int_left_shift(4);
    _data[0] += hex_char;
    return ret_val;

}

int bi::big_int::_big_int_get_hex_char_from_lsb(int hex_indx_from_lsb, BI_BASE_TYPE &hex_char) const {

    hex_char = 0;
    if (hex_indx_from_lsb < 0) {
        return -1;
    }

    /* hex_indx_from_lsb starts from 0. */
    int data_indx = hex_indx_from_lsb / (BI_BASE_TYPE_TOTAL_BITS / 4);
    int next_idx = hex_indx_from_lsb % (BI_BASE_TYPE_TOTAL_BITS / 4);

    if (data_indx >= _top) {
        return -1;
    }

    BI_BASE_TYPE temp_data = _data[data_indx];
    hex_char = (temp_data >> (next_idx * 4)) & 0xF;
    return 0;

}

int bi::big_int::_big_int_fast_modular_exponentiation(const bi::big_int &exponent, const bi::big_int &modulus, bi::big_int &result) {

    int ret_val = 0;

    if (exponent.big_int_is_zero() == true && modulus.big_int_is_negetive() == true) {
        big_int bi_1;
        bi_1.big_int_from_base_type(1, false);
        ret_val += modulus.big_int_unsigned_sub(bi_1, result);
        ret_val += result.big_int_set_negetive(true);
        return ret_val;
    }

    
    ret_val += result.big_int_from_base_type(1, false);
    big_int bi_2;
    ret_val += bi_2.big_int_from_base_type(2, false);
    
    big_int temp_base(*this), temp_exponent(exponent), temp_exponent_2, temp_result, temp_result_2;
    BI_BASE_TYPE temp_exponent_rem;
    while (temp_exponent.big_int_is_zero() == false) {
        ret_val += temp_exponent._big_int_fast_divide_by_two(temp_exponent_rem);
        if (temp_exponent_rem != 0) {
            ret_val += result.big_int_multiply(temp_base, temp_result);
            ret_val += temp_result.big_int_modulus(modulus, temp_result_2);
            result = temp_result_2;
        }
        ret_val += temp_base.big_int_multiply(temp_base, temp_result);
        ret_val += temp_result.big_int_modulus(modulus, temp_result_2);
        temp_base = temp_result_2;
    }
    return ret_val;

}

int bi::big_int::_big_int_fast_divide_by_two(BI_BASE_TYPE &remainder) {

    BI_DOUBLE_BASE_TYPE interim_res_1, interim_res_2;
    BI_BASE_TYPE        carry = 0;

    for (int i = _top - 1; i >= 0; --i) {
        interim_res_1 = static_cast<BI_DOUBLE_BASE_TYPE>(_data[i]) << BI_BASE_TYPE_TOTAL_BITS;
        interim_res_2 = (interim_res_1 >> 1);
        _data[i] = static_cast<BI_BASE_TYPE>((interim_res_2 & BI_DOUBLE_BASE_TYPE_FIRST_HALF_MASK) >> BI_BASE_TYPE_TOTAL_BITS) + carry;
        carry = interim_res_2 & BI_BASE_TYPE_MAX;
    }
    remainder = carry;
    return _big_int_remove_preceding_zeroes();

}

int bi::big_int::_big_int_generate_random_probable_prime(int bits, std::mt19937 &mt_arg, std::uniform_int_distribution<BI_BASE_TYPE> &uni_dist, int max_lower_prime_check) {

    int ret_val = 0;
    constexpr int max_prime_list_total_length = sizeof(first_primes_list) / sizeof(BI_BASE_TYPE);

    /* If max_lower_prime_check is negetive use all prime numbers in the array. */
    if (max_lower_prime_check < 0) {
        max_lower_prime_check = max_prime_list_total_length;
    }
    
    int max_prime_list_length = max_lower_prime_check;
    if (max_prime_list_length > max_prime_list_total_length) {
        max_prime_list_length = max_prime_list_total_length;
    }

    while (ret_val == 0) {
        int prim_cntr = 0;
        big_int rand_test_val, lower_prime, temp_quo, temp_rem;
        ret_val += rand_test_val._big_int_generate_random_unsigned(bits, mt_arg, uni_dist);
        for (int i = 0; i < max_prime_list_length; ++i) {
            ret_val += lower_prime.big_int_from_base_type(first_primes_list[i], false);
            ret_val += rand_test_val.big_int_div(lower_prime, temp_quo, temp_rem);
            if (temp_rem.big_int_is_zero() == true) {
                ++prim_cntr;
                break;
            }
        }
        if (prim_cntr == 0) {
            (*this) = rand_test_val;
            break;
        }
    }
    
    return ret_val;

}

int bi::big_int::_big_int_generate_random_unsigned(int bits, std::mt19937 &mt_arg, std::uniform_int_distribution<BI_BASE_TYPE> &uni_dist) {

    big_int_clear();

    if ((bits / BI_BASE_TYPE_TOTAL_BITS) >= _total_data) {
        _big_int_expand(BI_DEFAULT_EXPAND_COUNT + (bits / BI_BASE_TYPE_TOTAL_BITS));
    }

    for (int i = 0; i < bits / BI_BASE_TYPE_TOTAL_BITS; ++i) {
        _data[_top++] = uni_dist(mt_arg);
    }
    
    int rem_bits = bits % BI_BASE_TYPE_TOTAL_BITS;
    if (rem_bits > 0) {
        if (_top >= _total_data) {
            _big_int_expand(BI_DEFAULT_EXPAND_COUNT);
        }
        BI_BASE_TYPE temp_val = (uni_dist(mt_arg) % static_cast<BI_BASE_TYPE>((1 << rem_bits)));
        if (temp_val > 0) {
            _data[_top++] = temp_val;
        }
    }

    return 0;

}

int bi::big_int::_big_int_get_random_unsigned_between(
    std::mt19937 &mt_arg, 
    std::uniform_int_distribution<BI_BASE_TYPE> &uni_dist, 
    std::uniform_int_distribution<int> &uni_dist_rand_bits, 
    const big_int &low, 
    const big_int &high) {

    int ret_val = 0;

    if (high.big_int_unsigned_compare(low) < 0) {
        return -1;
    }

    int rand_bits = uni_dist_rand_bits(mt_arg);
    while (ret_val == 0) {
        big_int temp_rand;
        temp_rand._big_int_generate_random_unsigned(rand_bits, mt_arg, uni_dist);
        int low_comp_res = low.big_int_unsigned_compare(temp_rand);   
        int high_comp_res = high.big_int_unsigned_compare(temp_rand); 
        if ((low_comp_res <= 0) && (high_comp_res > 0)) {
            (*this) = temp_rand;
            break;
        }
    }

    return ret_val;

}