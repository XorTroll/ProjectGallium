
#pragma once

#ifdef __SWITCH__
    #include <switch.h>
    #define GA_ONCONSOLE 1
#else
    #include <cstdint>
    typedef uint8_t u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;
    #define GA_ONCONSOLE 0
#endif