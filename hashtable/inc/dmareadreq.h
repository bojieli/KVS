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
        ulong address;
        uint  size;
}
#ifdef _MSC_VER
#pragma pack(pop)
#endif
 req;
} DMA_ReadReq;

typedef struct __attribute__((packed)) DMA_ReadReq_Compressed {
  ulong address;
  uint size;
}DMA_ReadReq_Compressed;


typedef struct __attribute__((packed)) DMA_ReadReq_Extended {
  ulong address;
  uint size;
  bool should_swapin;
}DMA_ReadReq_Extended;
