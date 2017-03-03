inline uint hash_func1(const ulong4 *key) {
  uint hash = 0;
  uint kx = (key -> x >> 40) + (key -> x << 11);
  uint ky = (key -> y >> 30) + (key -> y << 23);
  uint kz = (key -> z >> 36) + (key -> z << 14);
  uint kw = (key -> w >> 27) + (key -> w << 17);

  hash += kx;
  hash += hash >> 11;
  hash += ky;
  hash += hash << 15;
  hash += kz;
  hash += hash << 3;
  hash += kw;
  hash += hash >> 7;

  return hash & ((1 << 30) - 1);
}

inline uint hash_func2(const ulong4 *key) {
  uint hash = 0;
  uint kx = (key -> x >> 4) + (key -> x << 7);
  uint ky = (key -> y >> 6) + (key -> y << 9);
  uint kz = (key -> z >> 7) + (key -> z << 11);
  uint kw = (key -> w >> 22) + (key -> w << 6);
  hash += kx;
  hash += hash << 8;
  hash += ky;
  hash += hash >> 10;
  hash += kz;
  hash += hash << 7;
  hash += kw;
  hash += hash >> 3;
  hash = (hash >> 16) + (hash & 0xFFFF);
  
  return hash & 1023;
}
