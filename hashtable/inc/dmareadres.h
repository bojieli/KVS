#pragma once

typedef union {
    ulong8 raw;

    
#ifdef _MSC_VER
#pragma pack(push,1)
struct 
#else
struct __attribute__((packed)) 
#endif
{ 
        ulong  address; // base address of the response
        uint   size;    // total size of the response
        uint   offset;  // byte offset of the flit in response
        ulong4 data;
     }
#ifdef _MSC_VER
#pragma pack(pop)
#endif
 res;
} DMA_ReadRes;