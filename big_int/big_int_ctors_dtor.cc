/** 
 *  @file   big_int_ctors_dtor.cc
 *  @brief  Big int constructors and destructor
 *
 *  Big int constructors and destructor definitions
 *
 *  @author         Tony Josi   https://tonyjosi97.github.io/profile/
 *  @copyright      Copyright (C) 2021 Tony Josi
 *  @bug            No known bugs.
 */


#include <stdexcept>

#include "big_int.hpp"
#include "big_int_lib_log.hpp"


bi::big_int::big_int() 
:   _total_data {DEFAULT_MEM_ALLOC_BYTES / sizeof(BI_BASE_TYPE)},  
    _top        {0},
    _neg        {false} {

    _data           = new BI_BASE_TYPE[_total_data];

    if(_data) {
        _data[_top++] = 0; /* Init. with zero. */
        _BI_LOG(1, "Big int init, with: %d items", _total_data);
    } else {
        _BI_LOG(1, "Init failed");
        throw std::length_error("Couldnt find enough memory");
    }

    _BI_LOG(3, "Default 'ctor");
}

bi::big_int::big_int(const bi::big_int &src) 
:   _total_data {src._total_data},  
    _top        {src._top},
    _neg        {src._neg} {

    _data           = new BI_BASE_TYPE[_total_data];

    if(_data) {
        _BI_LOG(1, "Big int init, with: %d items", _total_data);
    } else {
        _BI_LOG(1, "Init failed");
        throw std::length_error("Couldnt find enough memory");
    }

    for(int i = 0; i < _top; ++i) {
        _data[i] = src._data[i];
    }

    _BI_LOG(3, "Copy 'ctor");
}

bi::big_int& bi::big_int::operator=(bi::big_int src) {

    _big_int_swap(src);
    return *this;

    _BI_LOG(3, "Copy/Move assign. 'ctor");

}

bi::big_int::big_int(bi::big_int &&src)
:   _data       {nullptr},
    _total_data {0},  
    _top        {0},
    _neg        {false} {

    _big_int_swap(src);

    _BI_LOG(3, "Move 'ctor");

}

bi::big_int::~big_int() {

    delete[]    _data;
    _BI_LOG(1, "Freeing: %d, items", _total_data);

}

