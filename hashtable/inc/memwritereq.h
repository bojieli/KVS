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
  bool is_32B;
  bool is_first_half;
}Mem_WriteReq_Tmp;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

typedef struct __attribute__((packed)) Mem_WriteReq {
  ulong address;
  ulong8 data;
  ulong flag;
}Mem_WriteReq;
