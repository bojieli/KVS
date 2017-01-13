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


_CL_VOID
array_req_generator() {
  ArrayGetReqInfo info;
  info.cnt = 0;
  ushort cnt = 0;
  uchar key_slice[32];
  while (1) {
    if (info.cnt == cnt) {
      bool read_array_req_info;
      info = read_channel_nb_altera(array_req_info, &read_array_req_info);
      cnt = 0;
      if (!read_array_req_info) {
	info.cnt = 0;
      }
      else {
#pragma unroll
	for (int i = 0; i < 8; i ++) {
	  key_slice[i] = ((info.key.x >> ((7 - i) << 3))) & 0xFF;
	  key_slice[i + 8] = ((info.key.y >> ((7 - i) << 3))) & 0xFF;
	  key_slice[i + 16] = ((info.key.z >> ((7 - i) << 3))) & 0xFF;
	  key_slice[i + 24] = ((info.key.w >> ((7 - i) << 3))) & 0xFF;
	}
      }
    }

    if (info.cnt != cnt) {
      cnt ++;
      uchar cnt_in_char_size;
      uchar cnt_in_char[16];
      if (cnt / 10ULL == 0) {
	cnt_in_char_size = 1;
	cnt_in_char[0] = cnt;
      }
      else if (cnt / 100ULL == 0) {
	cnt_in_char_size = 2;
	cnt_in_char[1] = cnt % 10;
	cnt_in_char[0] = cnt / 10;
      }
      else if (cnt / 1000ULL == 0) {
	cnt_in_char_size = 3;
	cnt_in_char[2] = cnt % 10;
	cnt_in_char[1] = (cnt / 10) % 10;
	cnt_in_char[0] = cnt / 100;
      }
      else if (cnt / 10000ULL == 0) {
	cnt_in_char_size = 4;
	cnt_in_char[3] = cnt % 10;
	cnt_in_char[2] = (cnt / 10) % 10;
	cnt_in_char[1] = (cnt / 100) % 10;
	cnt_in_char[0] = cnt / 1000;
      }
      else {
	cnt_in_char_size = 5;
	cnt_in_char[4] = cnt % 10;
	cnt_in_char[3] = (cnt / 10) % 10;
	cnt_in_char[2] = (cnt / 100) % 10;
	cnt_in_char[1] = (cnt / 1000) % 10;
	cnt_in_char[0] = cnt / 10000;
      }
      
      GetReq req;
      req.is_array_first = false;
      req.net_meta = info.net_meta;
      req.key_size = info.key_size + 1;
#pragma unroll
      for (int i = 0; i < 32; i ++) {
	if (i >= info.key_size && i - info.key_size < cnt_in_char_size) {
	  key_slice[i] = cnt_in_char[i - info.key_size] + '0';
	}
      }
      ulong tmp[4];
#pragma unroll
      for (int i = 0; i < 4; i ++) {
	tmp[i] =
	  ((ulong)key_slice[0 + (i << 3)] << 56) |
	  ((ulong)key_slice[1 + (i << 3)] << 48) |
	  ((ulong)key_slice[2 + (i << 3)] << 40) |
	  ((ulong)key_slice[3 + (i << 3)] << 32) |
	  ((ulong)key_slice[4 + (i << 3)] << 24) |
	  ((ulong)key_slice[5 + (i << 3)] << 16) |
	  ((ulong)key_slice[6 + (i << 3)] << 8) |
	  ((ulong)key_slice[7 + (i << 3)])
	  ;
      }
      req.key.x = tmp[0];
      req.key.y = tmp[1];
      req.key.z = tmp[2];
      req.key.w = tmp[3];
      req.hash1 = hash_func1(&req.key);
      req.hash2 = hash_func2(&req.key);
      
      bool dummy = write_channel_nb_altera(array_get_req, req);
      assert(dummy);
    }
  }
}
