#pragma once

typedef union {
    ulong4 raw;
#ifdef _MSC_VER
#pragma pack(push,1)
struct 
#else
struct __attribute__((packed)) 
#endif
{ 
        ulong4 data;
}
#ifdef _MSC_VER
#pragma pack(pop)
#endif
 res;
} DMA_ReadRes;
