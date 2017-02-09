_CL_VOID
hashtable_get_array_req_generator() {
  ArrayGetReqInfo info;
  info.cnt = 0;
  ushort cnt = 0;
  uchar key_slice[32];
  while (1) {
    if (info.cnt == cnt) {
      bool read_array_req_info;
      info = read_channel_nb_altera(array_get_req_info, &read_array_req_info);
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

      ushort d0 = cnt;
      ushort d1 = d0 >> 4;
      ushort d2 = d1 >> 4;
      ushort d3 = d2 >> 4;

      ushort m0 = d0 & 15;
      ushort m1 = d1 & 15;
      ushort m2 = d2 & 15;
      ushort m3 = d3 & 15;      
      
      if (d1 == 0) {
	cnt_in_char_size = 1;
	cnt_in_char[0] = d0;
      }
      else if (d2 == 0) {
	cnt_in_char_size = 2;
	cnt_in_char[1] = m0;
	cnt_in_char[0] = d1;
      }
      else if (d3 == 0) {
	cnt_in_char_size = 3;
	cnt_in_char[2] = m0;
	cnt_in_char[1] = m1;
	cnt_in_char[0] = d2;
      }
      else {
	cnt_in_char_size = 4;
	cnt_in_char[3] = m0;
	cnt_in_char[2] = m1;
	cnt_in_char[1] = m2;
	cnt_in_char[0] = d3;
      }

      GetReq req;
      req.is_array_first = false;
      req.net_meta = info.net_meta;
      req.key_size = info.key_size + 1;

#define unroll_sec(idx)							\
      if (idx == info.key_size && i >= idx && i - idx < cnt_in_char_size) { \
	key_slice[i] = cnt_in_char[i - idx] + '0';			\
      }
      
#pragma unroll
      for (int i = 0; i < 32; i ++) {
	UNROLL_2_to_31;
      }
#undef unroll_sec      
      
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
