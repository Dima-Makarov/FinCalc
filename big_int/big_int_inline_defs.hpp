/** 
 *  @file   big_int_inline_defs.hpp
 *  @brief  Header with inline function definitions  
 *
 *  Header with inline function definitions  
 *
 *  @author         Tony Josi   https://tonyjosi97.github.io/profile/
 *  @copyright      Copyright (C) 2021 Tony Josi
 *  @bug            No known bugs.
 */

#include "big_int.hpp"

#pragma once

static inline int compare_bi_base_type(const BI_BASE_TYPE a, const BI_BASE_TYPE b) {

    if(a >= b)
        return 1;
    else
        return 0;

}

static inline int compare_bi_base_type_n_top(const BI_BASE_TYPE a, const BI_BASE_TYPE b, const int a_top, const int b_top) {

    if (a_top == b_top) {
        return compare_bi_base_type(a, b);
    } else if (a_top > b_top) {
        return 1;
    } else {
        return 0;
    }

}