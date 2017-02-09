_CL_VOID
hashtable_add_adder() {
  ushort val_size_left = 0;
  uchar net_meta;
  uchar key_size;
  ulong4 key;
  ulong base_addr;
  bool is_array;
  bool is_array_first;
  uchar wr_buf[64];
  uchar wr_buf_idx = 0;
  uint delta;
  ulong val_half;
  uchar wr_cnt = 0;
  bool last_write = false;
  
  while (1) {
    bool read_add_offline_parsed;
    DMA_WriteReq wr_req;
    bool should_write_hashtable_add_offline_update_slab_dma_wr_req = false;

    bool should_write_add_offline_res = false;
    AddRes val_write_add_offline_res;

    uchar val_in_uchar[32];
    uchar val_in_uchar_idx;
    uint val_in_uint[8];
    ulong wr_buf_first_32_in_ulong[4];
    
    if (last_write) {
      last_write = false;
#pragma unroll
      for (int i = 0; i < 4; i ++) {
	wr_buf_first_32_in_ulong[i] =
	  ((ulong)wr_buf[0]) << 56 |
	  ((ulong)wr_buf[1]) << 48 |
	  ((ulong)wr_buf[2]) << 40 |
	  ((ulong)wr_buf[3]) << 32 |
	  ((ulong)wr_buf[4]) << 24 |
	  ((ulong)wr_buf[5]) << 16 |
	  ((ulong)wr_buf[6]) << 8 |
	  ((ulong)wr_buf[7]) << 0;
      }
      should_write_hashtable_add_offline_update_slab_dma_wr_req = true;
      wr_req.req.size = 32;
      wr_req.req.address = base_addr + (wr_cnt << 5);
      wr_req.req.data.x = wr_buf_first_32_in_ulong[0];
      wr_req.req.data.y = wr_buf_first_32_in_ulong[1];
      wr_req.req.data.z = wr_buf_first_32_in_ulong[2];
      wr_req.req.data.w = wr_buf_first_32_in_ulong[3];
      wr_cnt = 0;

      if (is_array_first || !is_array) {
	AddRes res;
	res.net_meta = net_meta;
	res.found = true;
	res.key_size = key_size;
	res.key = key;
	val_write_add_offline_res = res;
	should_write_add_offline_res = true;
      }
    }
    else {
      AddOfflineParsed parsed = read_channel_nb_altera(add_offline_parsed, &read_add_offline_parsed);
      if (read_add_offline_parsed) {
	if (!val_size_left) {
	  net_meta = parsed.net_meta;
	  key_size = parsed.key_size;
	  key = parsed.key;
	  val_size_left = parsed.val_size;	
	  base_addr = parsed.base_addr;
	  is_array_first = parsed.is_array_first;
	  is_array = parsed.is_array;
	  delta = parsed.delta;
	  ushort size = (((ushort)parsed.key_size) << 11) | parsed.val_size;
	  wr_buf[0] = size >> 8;
	  wr_buf[1] = size & 0xFF;
	  ulong key_in_ulong[4];
	  key_in_ulong[0] = parsed.key.x;
	  key_in_ulong[1] = parsed.key.y;
	  key_in_ulong[2] = parsed.key.z;
	  key_in_ulong[3] = parsed.key.w;
	  uchar key_in_uchar[32];
	
#pragma unroll
	  for (int i = 0; i < 8; i ++) {
	    key_in_uchar[i] = (key_in_ulong[0] >> ((7 - i) << 3)) & 0xFF;
	    key_in_uchar[i + 8] = (key_in_ulong[1] >> ((7 - i) << 3)) & 0xFF;
	    key_in_uchar[i + 16] = (key_in_ulong[2] >> ((7 - i) << 3)) & 0xFF;
	    key_in_uchar[i + 24] = (key_in_ulong[3] >> ((7 - i) << 3)) & 0xFF;
	  }
	  if (is_array_first) {
	    val_in_uint[0] = ((parsed.val.x >> 16) & 0xFFFFFFFF) + delta;	  
	    val_in_uint[1] = (((parsed.val.x & 0xFFFF) << 16) | (parsed.val.y >> 48)) + delta;
	    val_in_uint[2] = ((parsed.val.y >> 16) & 0xFFFFFFFF) + delta;
	    val_in_uint[3] = (((parsed.val.y & 0xFFFF) << 16) | (parsed.val.z >> 48)) + delta;
	    val_in_uint[4] = ((parsed.val.z >> 16) & 0xFFFFFFFF) + delta;
	    val_in_uint[5] = (((parsed.val.z & 0xFFFF) << 16) | (parsed.val.w >> 48)) + delta;
	    val_in_uint[6] = ((parsed.val.w >> 16) & 0xFFFFFFFF) + delta;
	    val_half = parsed.val.w & 0xFFFF;
	  
#pragma unroll
	    for (int i = 0; i < 7; i ++) {
	      val_in_uchar[0 + (i << 2)] = (val_in_uint[i] >> 24) & 0xFF;
	      val_in_uchar[1 + (i << 2)] = (val_in_uint[i] >> 16) & 0xFF;
	      val_in_uchar[2 + (i << 2)] = (val_in_uint[i] >> 8) & 0xFF;
	      val_in_uchar[3 + (i << 2)] = (val_in_uint[i] >> 0) & 0xFF;
	    }
	    val_in_uchar[28] = val_in_uchar[29] = val_in_uchar[30] = val_in_uchar[31] = 0;
	    val_in_uchar_idx = (30 > (val_size_left - 2)) ? (val_size_left - 2) : 30;
	  }
	  else {
	    val_in_uint[0] = ((parsed.val.x >> 32) & 0xFFFFFFFF) + delta;
	    val_in_uint[1] = ((parsed.val.x >> 0) & 0xFFFFFFFF) + delta;
	    val_in_uint[2] = ((parsed.val.y >> 32) & 0xFFFFFFFF) + delta;
	    val_in_uint[3] = ((parsed.val.y >> 0) & 0xFFFFFFFF) + delta;
	    val_in_uint[4] = ((parsed.val.z >> 32) & 0xFFFFFFFF) + delta;
	    val_in_uint[5] = ((parsed.val.z >> 0) & 0xFFFFFFFF) + delta;
	    val_in_uint[6] = ((parsed.val.w >> 32) & 0xFFFFFFFF) + delta;
	    val_in_uint[7] = ((parsed.val.w >> 0) & 0xFFFFFFFF) + delta;
	  
#pragma unroll
	    for (int i = 0; i < 8; i ++) {
	      val_in_uchar[0 + (i << 2)] = (val_in_uint[i] >> 24) & 0xFF;
	      val_in_uchar[1 + (i << 2)] = (val_in_uint[i] >> 16) & 0xFF;
	      val_in_uchar[2 + (i << 2)] = (val_in_uint[i] >> 8) & 0xFF;
	      val_in_uchar[3 + (i << 2)] = (val_in_uint[i] >> 0) & 0xFF;
	    }
	    val_in_uchar_idx = (32 > val_size_left) ? (val_size_left) : 32;
	  }
	
#pragma unroll
	  for (int i = 0; i < 64; i ++) {
	    if (i >= 2 && i < 2 + parsed.key_size) {
	      wr_buf[i] = key_in_uchar[i - 2];
	    }
	  }
	  if (is_array_first) {
#pragma unroll
	    for (int i = 0; i < 64; i ++) {
	      if (i == 2 + parsed.key_size) {
		wr_buf[i] = (parsed.val.x >> 56) & 0xFF;
	      }
	      else if (i == 3 + parsed.key_size) {
		wr_buf[i] = (parsed.val.x >> 48) & 0xFF;
	      }
	      else if (i >= 4 + parsed.key_size && i < 4 + parsed.key_size + val_in_uchar_idx) {
		wr_buf[i] = val_in_uchar[i - (4 + parsed.key_size)];
	      }
	    }
	  }
	  else {

#pragma unroll
	    for (int i = 0; i < 64; i ++) {
	      if (i >= 2 + parsed.key_size && i < 2 + parsed.key_size + val_in_uchar_idx) {
		wr_buf[i] = val_in_uchar[i - (2 + parsed.key_size)];
	      }
	    }	  
	  }
	  if (val_size_left >= 32) {
	    wr_buf_idx = 2 + parsed.key_size + (32 - (is_array_first << 1));
	  }
	  else {	
	    wr_buf_idx = 2 + parsed.key_size + val_size_left;
	  }
	  if (val_size_left > 32) {
	    val_size_left -= 32 - (is_array_first << 1);
	  }
	  else {
	    val_size_left = 0;
	    last_write = wr_buf_idx;
	  }
	}
	else {
	  if (is_array_first) {
	    val_in_uint[0] = (((val_half) << 16) | (parsed.val.x >> 48)) + delta;
	    val_in_uint[1] = ((parsed.val.x >> 16) & 0xFFFFFFFF) + delta;
	    val_in_uint[2] = (((parsed.val.x & 0xFFFF) << 16) | (parsed.val.y >> 48)) + delta;
	    val_in_uint[3] = ((parsed.val.y >> 16) & 0xFFFFFFFF) + delta;
	    val_in_uint[4] = (((parsed.val.y & 0xFFFF) << 16) | (parsed.val.z >> 48)) + delta;
	    val_in_uint[5] = ((parsed.val.z >> 16) & 0xFFFFFFFF) + delta;
	    val_in_uint[6] = (((parsed.val.z & 0xFFFF) << 16) | (parsed.val.w >> 48)) + delta;
	    val_in_uint[7] = ((parsed.val.w >> 16) & 0xFFFFFFFF) + delta;
	    val_half = parsed.val.w & 0xFFFF;
	  
#pragma unroll
	    for (int i = 0; i < 8; i ++) {
	      val_in_uchar[0 + (i << 2)] = (val_in_uint[i] >> 24) & 0xFF;
	      val_in_uchar[1 + (i << 2)] = (val_in_uint[i] >> 16) & 0xFF;
	      val_in_uchar[2 + (i << 2)] = (val_in_uint[i] >> 8) & 0xFF;
	      val_in_uchar[3 + (i << 2)] = (val_in_uint[i] >> 0) & 0xFF;
	    }
	  }
	  else {
	    val_in_uint[0] = ((parsed.val.x >> 32) & 0xFFFFFFFF) + delta;
	    val_in_uint[1] = ((parsed.val.x >> 0) & 0xFFFFFFFF) + delta;
	    val_in_uint[2] = ((parsed.val.y >> 32) & 0xFFFFFFFF) + delta;
	    val_in_uint[3] = ((parsed.val.y >> 0) & 0xFFFFFFFF) + delta;
	    val_in_uint[4] = ((parsed.val.z >> 32) & 0xFFFFFFFF) + delta;
	    val_in_uint[5] = ((parsed.val.z >> 0) & 0xFFFFFFFF) + delta;
	    val_in_uint[6] = ((parsed.val.w >> 32) & 0xFFFFFFFF) + delta;
	    val_in_uint[7] = ((parsed.val.w >> 0) & 0xFFFFFFFF) + delta;
	  
#pragma unroll
	    for (int i = 0; i < 8; i ++) {
	      val_in_uchar[0 + (i << 2)] = (val_in_uint[i] >> 24) & 0xFF;
	      val_in_uchar[1 + (i << 2)] = (val_in_uint[i] >> 16) & 0xFF;
	      val_in_uchar[2 + (i << 2)] = (val_in_uint[i] >> 8) & 0xFF;
	      val_in_uchar[3 + (i << 2)] = (val_in_uint[i] >> 0) & 0xFF;
	    }
	  }	

	  ushort new_wr_buf_idx;
	  if (val_size_left >= 32) {
	    new_wr_buf_idx = wr_buf_idx + 32;
	  }
	  else {
	    new_wr_buf_idx = wr_buf_idx + val_size_left;
	  }

#pragma unroll
	  for (int i = 0; i < 64; i ++) {
	    if (i >= wr_buf_idx && i < new_wr_buf_idx) {
	      wr_buf[i] = val_in_uchar[i - wr_buf_idx];
	    }
	  }	
	  wr_buf_idx = new_wr_buf_idx;
	  if (val_size_left > 32) {
	    val_size_left -= 32;
	  }
	  else {
	    val_size_left = 0;
	    last_write = wr_buf_idx;
	  }
	}
      
#pragma unroll
	for (int i = 0; i < 4; i ++) {
	  wr_buf_first_32_in_ulong[i] =
	    ((ulong)wr_buf[0 + (i << 3)]) << 56 |
	    ((ulong)wr_buf[1 + (i << 3)]) << 48 |
	    ((ulong)wr_buf[2 + (i << 3)]) << 40 |
	    ((ulong)wr_buf[3 + (i << 3)]) << 32 |
	    ((ulong)wr_buf[4 + (i << 3)]) << 24 |
	    ((ulong)wr_buf[5 + (i << 3)]) << 16 |
	    ((ulong)wr_buf[6 + (i << 3)]) << 8 |
	    ((ulong)wr_buf[7 + (i << 3)]) << 0;
	}
	if (wr_buf_idx > 32) {
	  wr_buf_idx -= 32;
	}
	else {
	  wr_buf_idx = 0;
	}
	should_write_hashtable_add_offline_update_slab_dma_wr_req = true;
	wr_req.req.size = 32;
	wr_req.req.address = base_addr + (wr_cnt << 5);
	wr_req.req.data.x = wr_buf_first_32_in_ulong[0];      
	wr_req.req.data.y = wr_buf_first_32_in_ulong[1];
	wr_req.req.data.z = wr_buf_first_32_in_ulong[2];
	wr_req.req.data.w = wr_buf_first_32_in_ulong[3];
	wr_cnt ++;
      
#pragma unroll
	for (int i = 0; i < 64; i ++) {
	  if (i < 32) {
	    wr_buf[i] = wr_buf[i + 32];
	  }
	  else {
	    wr_buf[i] = 0;
	  }
	}
      }
    }

    if (should_write_hashtable_add_offline_update_slab_dma_wr_req) {
      DMA_WriteReq_Compressed wr_req_compressed;
      wr_req_compressed.address = wr_req.req.address;
      wr_req_compressed.size = wr_req.req.size;
      wr_req_compressed.data = wr_req.req.data;
      
      bool dummy = write_channel_nb_altera(hashtable_add_offline_update_slab_dma_wr_req, wr_req_compressed);
      assert(dummy);
    }

    if (should_write_add_offline_res) {
      bool dummy = write_channel_nb_altera(add_offline_res, val_write_add_offline_res);
      assert(dummy);
    }
    
  }
}

