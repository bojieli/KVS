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
}DMA_WriteReq;   

typedef struct __attribute__((packed)) DMA_WriteReq_Compressed {
  ulong address;
  uint size;
  ulong4 data;
}DMA_WriteReq_Compressed;

typedef struct __attribute__((packed)) DMA_WriteReq_Compressed_Double {
  DMA_WriteReq_Compressed x;
  DMA_WriteReq_Compressed y;
  bool valid2;
}DMA_WriteReq_Compressed_Double;

