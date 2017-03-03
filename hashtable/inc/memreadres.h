#pragma once

#ifdef _MSC_VER
#pragma pack(push,1)
typedef struct 
#else
typedef struct __attribute__((packed)) 
#endif
{ 
	ulong8 data;
	ulong flag;
}Mem_ReadRes;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

