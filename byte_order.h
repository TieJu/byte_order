/**
 * Author: Tiemo Jung
 * contact: tiemo dot jung at mni dot fh-giessen dot de
 * licencse: zlib Licencse
 * Copyright (c) 2012 Tiemo Jung
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *  claim that you wrote the original software. If you use this software
 *  in a product, an acknowledgment in the product documentation would be
 *  appreciated but is not required.
 *
 *  2. Altered source versions must be plainly marked as such, and must not be
 *  misrepresented as being the original software.
 *
 *  3. This notice may not be removed or altered from any source
 *  distribution.
 */
#ifndef __BYTE_ORDER_H__
#define __BYTE_ORDER_H__

#include <algorithm>

namespace byte_order {
/**
 * Tag type for any ordering type
 */
struct any_ordering {};
/**
 * Tag type for keeping the element ordering (native).
 */
struct keep_ordering    : any_ordering {};
/**
 * Tag type for reversing the element ordering.
 */
struct reverse_ordering : any_ordering {};

#if defined(BIG_ENDIAN)
typedef reverse_ordering        little_endian_ordering;
typedef keep_ordering           big_endian_ordering;
#else
typedef keep_ordering           little_endian_ordering;
typedef reverse_ordering        big_endian_ordering;
#endif

typedef big_endian_ordering     net_ordering;

namespace detail {
template<typename OuterType,typename InnerType>
inline void reorder(OuterType &,keep_ordering ) {
    //(void)in_out;
}

template<typename InnerType,typename OuterType>
inline void reorder(OuterType &out,const OuterType in,keep_ordering ) {
    out = in;
}

template<typename InnerType,size_t length>
inline void reorder(InnerType *in_out,reverse_ordering ) {
    std::reverse(in_out,in_out + length);
}

template<typename InnerType,typename OuterType>
inline void reorder(OuterType &in_out,reverse_ordering ) {
    auto begin  = reinterpret_cast<InnterType *>(&in);
    auto end    = begin + (sizeof(OuterType) / sizeof(InnerType));
    std::reverse(begin,end);
}

template<typename InnerType,typename OuterType>
inline void reorder(OuterType &out,const OuterType in,reverse_ordering ) {
    auto begin      = reinterpret_cast<InnterType *>(&in);
    auto end        = begin + (sizeof(OuterType) / sizeof(InnerType));
    auto out_begin  = reinterpret_cast<InnerType *>(&out);
    std::reverse_copy(begin,end,out_begin);
}

template<typename InnerType,typename OuterType,typename Ordering>
inline OuterType reorder_copy(OuterType in,Ordering o) {
    reorder<OuterType,InnerType>(in,o);
    return in;
}

inline bool is_native(keep_ordering ) {
    return true;
}

inline bool is_native(any_ordering ) {
    return false;
}
}

/**
 * Represents an ordered data type.
 * OuterType is the type on which the ordering is applyed.
 * OrderingType is the type tag which selects the ordering mode.
 * InnnerType is the type on which level (typicaly byte (unsigned char) level) the reordering
 * should be applyed. This enables 'mixed' ordering via nested ordering types.
 * Any integer and floating point type that is wrapped with this, can safly used for
 * auto translations of types.
 *
 * Example:
 * 
 * insted of:
 * struct my_file_header {
 * int first_int;
 * int second_int;
 * };
 * void load(void *data,my_file_header *&hdr) {
 *	hdr = reinterpret_cast<my_file_header>(data);
 *  reorder_from_big(hdr.first_int);
 *  reorder_from_little(hdr.second_int);
 * }
 *
 * you can write:
 * struct my_file_header {
 * big_int_t first_int;
 * little_int_t second_int;
 * };
 * void load(void *data,my_file_header *&hdr) {
 *	hdr = reinterpret_cast<my_file_header>(data);
 * }
 * This avoids errors of missing type conversations.
 * Drawback is, each time a type is accessed which has not the system endianess
 * it will be reordered, intead on the reordering phase.
 */
template<typename OuterType
        ,typename OrderingType
        ,typename InnerType = unsigned char
        >
struct ordered_type {
    OuterType   data;

    ordered_type(const OuterType &init = OuterType()) {
        detail::reorder<InnerType>(this->data,init,OrderingType());
    }

    ordered_type(const ordered_type &other)
        : data(other.data) {
    }

    ordered_type &operator=(const ordered_type &other) {
        if ( this != &other ) {
            this->data = other.data;
        }
        return *this;
    }

    ordered_type &operator=(const OuterType &init) {
        detail::reorder<InnerType>(this->data,init,OrderingType());
        return *this;
    }

    operator OuterType() const {
        return detail::reorder_copy<InnerType>(this->data,OrderingType());
    }

    OuterType raw_get() const {
        return this->data;
    }

    void raw_set(const OuterType &b) {
        this->data = b;
    }

    static bool is_native() { return detail::is_native(OrderingType()); }
};

/**
 * Predef of all base types with big, little and net ordering.
 */

typedef ordered_type<char,big_endian_ordering>                      big_char_t,             big_char;
typedef ordered_type<unsigned char,big_endian_ordering>             big_uchar_t,            big_uchar;
typedef ordered_type<signed char,big_endian_ordering>               big_schar_t,            big_schar;
typedef ordered_type<short,big_endian_ordering>                     big_short_t,            big_short;
typedef ordered_type<unsigned short,big_endian_ordering>            big_ushort_t,           big_ushort;
typedef ordered_type<signed short,big_endian_ordering>              big_sshort_t,           big_sshort;
typedef ordered_type<long,big_endian_ordering>                      big_long_t,             big_long;
typedef ordered_type<unsigned long,big_endian_ordering>             big_ulong_t,            big_ulong;
typedef ordered_type<signed long,big_endian_ordering>               big_slong_t,            big_slong;
typedef ordered_type<int,big_endian_ordering>                       big_int_t,              big_int;
typedef ordered_type<unsigned int,big_endian_ordering>              big_uint_t,             big_uint;
typedef ordered_type<signed int,big_endian_ordering>                big_sint_t,             big_sint;
typedef ordered_type<long long,big_endian_ordering>                 big_long_long_t,        big_long_long;
typedef ordered_type<unsigned long long,big_endian_ordering>        big_ulong_long_t,       big_ulong_long;
typedef ordered_type<signed long long,big_endian_ordering>          big_slong_long_t,       big_slong_long;
typedef ordered_type<float,big_endian_ordering>                     big_float_t,            big_float;
typedef ordered_type<double,big_endian_ordering>                    big_double_t,           big_double;
typedef ordered_type<long double,big_endian_ordering>               big_long_double_t,      big_long_double;

typedef ordered_type<char,little_endian_ordering>                   little_char_t,          little_char;
typedef ordered_type<unsigned char,little_endian_ordering>          little_uchar_t,         little_uchar;
typedef ordered_type<signed char,little_endian_ordering>            little_schar_t,         little_schar;
typedef ordered_type<short,little_endian_ordering>                  little_short_t,         little_short;
typedef ordered_type<unsigned short,little_endian_ordering>         little_ushort_t,        little_ushort;
typedef ordered_type<signed short,little_endian_ordering>           little_sshort_t,        little_sshort;
typedef ordered_type<long,little_endian_ordering>                   little_long_t,          little_long;
typedef ordered_type<unsigned long,little_endian_ordering>          little_ulong_t,         little_ulong;
typedef ordered_type<signed long,little_endian_ordering>            little_slong_t,         little_slong;
typedef ordered_type<int,little_endian_ordering>                    little_int_t,           little_int;
typedef ordered_type<unsigned int,little_endian_ordering>           little_uint_t,          little_uint;
typedef ordered_type<signed int,little_endian_ordering>             little_sint_t,          little_sint;
typedef ordered_type<long long,little_endian_ordering>              little_long_long_t,     little_long_long;
typedef ordered_type<unsigned long long,little_endian_ordering>     little_ulong_long_t,    little_ulong_long;
typedef ordered_type<signed long long,little_endian_ordering>       little_slong_long_t,    little_slong_long;
typedef ordered_type<float,little_endian_ordering>                  little_float_t,         little_float;
typedef ordered_type<double,little_endian_ordering>                 little_double_t,        little_double;
typedef ordered_type<long double,little_endian_ordering>            little_long_double_t,   little_long_double;

typedef ordered_type<char,net_ordering>                             net_char_t,             net_char;
typedef ordered_type<unsigned char,net_ordering>                    net_uchar_t,            net_uchar;
typedef ordered_type<signed char,net_ordering>                      net_schar_t,            net_schar;
typedef ordered_type<short,net_ordering>                            net_short_t,            net_short;
typedef ordered_type<unsigned short,net_ordering>                   net_ushort_t,           net_ushort;
typedef ordered_type<signed short,net_ordering>                     net_sshort_t,           net_sshort;
typedef ordered_type<long,net_ordering>                             net_long_t,             net_long;
typedef ordered_type<unsigned long,net_ordering>                    net_ulong_t,            net_ulong;
typedef ordered_type<signed long,net_ordering>                      net_slong_t,            net_slong;
typedef ordered_type<int,net_ordering>                              net_int_t,              net_int;
typedef ordered_type<unsigned int,net_ordering>                     net_uint_t,             net_uint;
typedef ordered_type<signed int,net_ordering>                       net_sint_t,             net_sint;
typedef ordered_type<long long,net_ordering>                        net_long_long_t,        net_long_long;
typedef ordered_type<unsigned long long,net_ordering>               net_ulong_long_t,       net_ulong_long;
typedef ordered_type<signed long long,net_ordering>                 net_slong_long_t,       net_slong_long;
typedef ordered_type<float,net_ordering>                            net_float_t,            net_float;
typedef ordered_type<double,net_ordering>                           net_double_t,           net_double;
typedef ordered_type<long double,net_ordering>                      net_long_double_t,      net_long_double;
}

#endif//__BYTE_ORDER_H__