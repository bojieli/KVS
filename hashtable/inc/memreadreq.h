#pragma once

#ifdef _MSC_VER
#pragma pack(push,1)
typedef struct 
#else
typedef struct __attribute__((packed)) 
#endif
{ 
    uint address;
}Mem_ReadReq;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

