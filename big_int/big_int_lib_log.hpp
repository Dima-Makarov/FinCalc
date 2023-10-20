/** 
 *  @file   big_int_lib_log.hpp
 *  @brief  Header with macro definition for logging 
 *
 *  Header with macro definition for logging 
 *
 *  @author         Tony Josi   https://tonyjosi97.github.io/profile/
 *  @copyright      Copyright (C) 2021 Tony Josi
 *  @bug            No known bugs.
 */

#pragma once

#define         DEBUG_LOG       (0)

#if DEBUG_LOG

    #include <stdio.h>
    
    #define         _MAX_VERBOSE     (1)     /* Verbose level 3 - max,  1 - min */

    #define         _BI_LOG(_VERB, __str__, ...)    do {    \
        if(_VERB <= _MAX_VERBOSE) {                         \
            printf("\t>>> BI_LOG: ");                       \
            printf(__str__, ##__VA_ARGS__);                 \
            printf("\n");                                   \
        }                                                   \
        else {}                                             \
    } while(0)

#else   /* DEBUG_LOG */

    #define         _BI_LOG(_VERB, __str__, ...)     do {   \
        /* No debug log */                                  \
        } while(0)

#endif  /* DEBUG_LOG */
