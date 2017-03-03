// NOTE: should be awareness of 32B-write

#define exactAddr (address - offset)

// 25bits + 0 + 10bits

inline bool should_cache(ulong address, ulong offset) {
  return ((exactAddr >> 10) & 1) == 0;
}

inline ushort cache_slice_boundry_distance(ulong address, ulong offset) {
  return (((exactAddr + (1 << 10)) >> 10) << 10) - exactAddr;
}

inline uchar get_cache_flag(ulong address, ulong offset) {
  return (exactAddr >> (7 + 26));
}

inline uint get_cache_address(ulong address, ulong offset) {
  return (((exactAddr >> 11) & 0x3FFFFF) << 4) | ((exactAddr >> 6) & 0xF);
}

inline ulong get_pcie_address(uchar flag, ulong address, ulong offset) {
  return ((((ulong)flag) << 33) | ((address >> 4) << 11) | ((address & 0xF) << 6)) + offset;
}

#undef exactAddr
