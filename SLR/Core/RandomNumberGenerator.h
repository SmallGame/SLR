//
//  RandomNumberGenerator.h
//
//  Created by 渡部 心 on 2015/06/28.
//  Copyright (c) 2015年 渡部 心. All rights reserved.
//

#ifndef __SLR_RandomNumberGenerator__
#define __SLR_RandomNumberGenerator__

#include "../defines.h"
#include "../references.h"

struct Types32bit {
    typedef int32_t Int;
    typedef uint32_t UInt;
    typedef float Float;
};

struct Types64bit {
    typedef int64_t Int;
    typedef uint64_t UInt;
    typedef double Float;
};

template <typename TypeSet>
class RandomNumberGeneratorTemplate {
public:
    virtual typename TypeSet::UInt getUInt() = 0;
    
    virtual typename TypeSet::Float getFloat0cTo1o() {
        unsigned int rand23bit = (getUInt() >> 9) | 0x3f800000;
        return *(float*)&rand23bit - 1.0f;
    };
};

template <> Types32bit::Float RandomNumberGeneratorTemplate<Types32bit>::getFloat0cTo1o();
template <> Types64bit::Float RandomNumberGeneratorTemplate<Types64bit>::getFloat0cTo1o();

#endif