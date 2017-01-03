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
        bool    reserved;
        bool    eop;
        uchar   padbytes;
        ushort  slot;
        ulong4  data;
     }
#ifdef _MSC_VER
#pragma pack(pop)
#endif
 data;
} PCIE_Message;
