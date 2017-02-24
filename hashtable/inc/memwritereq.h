#pragma once

#ifdef _MSC_VER
#pragma pack(push,1)
typedef struct 
#else
typedef struct __attribute__((packed)) 
#endif
{ 
    ulong address;
	ulong8 data;
	ulong flag;
}Mem_WriteReq;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

