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
        ulong  address; // base address is only valid at first flit of the request 
        uint   size;    // request size is only valid at first flit, must be multiple of 32
        ulong4 data;
     }
#ifdef _MSC_VER
#pragma pack(pop)
#endif
 req;
} DMA_WriteReq;