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
        // Uplink message, the GID of the source kernel;
        // if downlink, GID of destination kernel
        ushort KID;

        // signal command
        ushort Cmd;

        // 4-byte integer parameter
        uint  SParam;

        // 8-byte integer parameter
        ulong LParam[7];
     }
#ifdef _MSC_VER
#pragma pack(pop)
#endif
 Sig;
} ClSignal;