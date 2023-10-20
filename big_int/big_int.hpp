/** 
 *  @file   big_int.hpp
 *  @brief  Main include file for the big int library
 *
 *  This file contains the source code the DS used as the big in library
 *
 *  @author         Tony Josi   https://tonyjosi97.github.io/profile/
 *  @copyright      Copyright (C) 2021 Tony Josi
 *  @bug            No known bugs.
 */

#include <stdint.h>
#include <string>
#include <random>

#pragma once

#define         BI_BASE_TYPE                                uint32_t
#define         BI_DOUBLE_BASE_TYPE                         uint64_t
#define         BI_SSCANF_FORMAT_HEX                        "%8X"
#define         BI_SPRINF_FORMAT_HEX                        "%08X"
#define         BI_SPRINF_FORMAT_DEC                        "%010u"
#define         BI_SPRINF_FORMAT_HEX_LOG                    "0x %08X"
#define         BI_SPRINF_FORMAT_DEC_LOG                    "d %010u"
#define         BI_SPRINF_FORMAT_HEX_CHARS                  8
#define         BI_SPRINF_FORMAT_DEC_CHARS                  10
#define         BI_HEX_STR_TO_DATA_SIZE                     (2 * sizeof(BI_BASE_TYPE))
#define         BI_DEFAULT_EXPAND_COUNT                     (32)
#define         BI_BASE_TYPE_MAX                            (0xFFFFFFFF)
#define         BI_BASE_TYPE_TOTAL_BITS                     (32)
#define         BI_DOUBLE_BASE_TYPE_FIRST_HALF_MASK         (0xFFFFFFFF00000000)
#define         BI_DOUBLE_BASE_TYPE_TOTAL_BITS              (64)

#define         DEFAULT_MEM_ALLOC_BYTES                     (128)

namespace bi {

    enum class bi_base {

        BI_BIN,
        BI_DEC,
        BI_HEX
    
    };

    class big_int {

        private:

        BI_BASE_TYPE    *_data;
        int             _total_data;
        int             _top;
        bool            _neg;

        int             _big_int_expand(int req);
        int             _big_int_from_string(const std::string &str_data);
        std::string     _big_int_to_string();
        BI_BASE_TYPE    _big_int_sub_base_type(BI_BASE_TYPE *data_ptr, int min, big_int &res_ptr) const;
        void            _big_int_swap(big_int &src);
        int             _big_int_compare_bi_base_type_n_top(const big_int &other) const;
        int             _big_int_unsigned_multiply_bi_base_type(BI_BASE_TYPE b, big_int &res_ptr) const;
        int             _big_int_left_shift_below_32bits(int bits);
        int             _big_int_right_shift_below_32bits(int bits);
        int             _big_int_remove_preceding_zeroes();
        int             _big_int_get_num_of_hex_chars() const;
        int             _big_int_divide_once(const big_int &divisor, BI_BASE_TYPE &op_quotient, big_int &op_remainder);
        int             _big_int_push_back_hex_chars(const BI_BASE_TYPE &hex_char);
        int             _big_int_get_hex_char_from_lsb(int hex_indx_from_lsb, BI_BASE_TYPE &hex_char) const;
        int             _big_int_fast_modular_exponentiation(const big_int &exponent, const big_int &modulus, big_int &result);
        int             _big_int_fast_divide_by_two(BI_BASE_TYPE &remainder);
        int             _big_int_generate_random_unsigned(int bits, std::mt19937 &mt_arg, std::uniform_int_distribution<BI_BASE_TYPE> &uni_dist);
        int             _big_int_get_random_unsigned_between(std::mt19937 &mt_arg, std::uniform_int_distribution<BI_BASE_TYPE> &uni_dist, \
            std::uniform_int_distribution<int> &uni_dist_rand_bits, const big_int &low, const big_int &high);

        
        

        public:

        big_int();
        big_int(const big_int &src);
        big_int& operator=(big_int src);
        big_int(big_int &&src);
        ~big_int();

        int             big_int_from_string(const std::string &str_num, bi_base target_base = bi_base::BI_HEX);
        int             big_int_from_base_type(const BI_BASE_TYPE &bt_val, const bool is_neg);
        std::string     big_int_to_string(bi_base target_base = bi_base::BI_HEX);
        int             big_int_compare(const big_int &other) const;
        int             big_int_unsigned_compare(const big_int &other) const;
        int             big_int_unsigned_add(const big_int &b);
        int             big_int_unsigned_add(const big_int &b, big_int &res);
        int             big_int_signed_add(const big_int &b);
        int             big_int_signed_add(const big_int &b, big_int &res);
        int             big_int_set_negetive(bool set_unset);
        bool            big_int_is_negetive() const;
        bool            big_int_is_zero() const;
        int             big_int_set_zero();
        int             big_int_clear();
        int             big_int_signed_sub(const big_int &b);
        int             big_int_signed_sub(const big_int &b, big_int &res);
        int             big_int_multiply(const big_int &b, big_int &res);
        int             big_int_unsigned_multiply_base_type(const BI_BASE_TYPE &b, big_int &res) const;
        int             big_int_get_num_of_hex_chars() const;
        int             big_int_get_num_of_bits() const;
        int             big_int_div(const big_int &divisor, big_int &quotient, big_int &remainder);
        int             big_int_power_base_type(const BI_BASE_TYPE &exponent, big_int &result);
        int             big_int_fast_modular_exponentiation(const big_int &exponent, const big_int &modulus, big_int &result);
        int             big_int_modulus(const big_int &modulus, big_int &result);
        int             big_int_gcd_euclidean_algorithm(const big_int &b, big_int &op_gcd);
        int             big_int_modular_inverse_extended_euclidean_algorithm(const big_int &modulus, big_int &inverse);
        bool            big_int_is_even() const;
        int             big_int_fast_divide_by_power_of_two(int power, big_int &remainder, big_int &coefficient) const;
        int             big_int_fast_multiply_by_power_of_two(int power, big_int &result) const;
        int             big_int_get_random_unsigned(int bits);
        int             big_int_get_random_unsigned_between(const big_int &low, const big_int &high);
        int             big_int_get_random_unsigned_prime_rabin_miller(int bits, int reqd_rabin_miller_iterations);
        int             big_int_get_random_unsigned_prime_rabin_miller_threaded(int bits, int reqd_rabin_miller_iterations, int no_of_threads);

        /* Logical shifts*/
        int             big_int_left_shift_word(int shift_words);
        int             big_int_left_shift_word(int shift_words, big_int &res);
        int             big_int_left_shift(int bits);
        int             big_int_left_shift(int bits, big_int &res);
        int             big_int_right_shift_word(int shift_words);
        int             big_int_right_shift_word(int shift_words, big_int &res);
        int             big_int_right_shift(int bits);
        int             big_int_right_shift(int bits, big_int &res);

        /* First param should be larger. */
        int             big_int_unsigned_sub(const big_int &b);
        int             big_int_unsigned_sub(const big_int &b, big_int &res) const;

        /* Temporary public func.s for testing */
        int             big_int_push_back_hex_chars(const BI_BASE_TYPE &hex_chars);
        int             big_int_divide_once(const big_int &divisor, BI_BASE_TYPE &op_quotient, big_int &op_remainder);


        /* TODO: make private */
        int             _big_int_generate_random_probable_prime(int bits, std::mt19937 &mt_arg, std::uniform_int_distribution<BI_BASE_TYPE> &uni_dist, int max_lower_prime_check);
        
    };

}
