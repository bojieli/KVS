_CL_VOID
hashtable_add_offline_handler() {
  ushort inflight_rd_res_size = 0;
  ushort val_size_left = 0;
  uchar val_res_idx = 0;
  uchar key_res_in_uchar[32];
  uchar val_res_in_uchar[32];
  ulong4 key;
  ushort key_size;
  ushort val_size;
  uchar net_meta;
  bool is_array;
  bool is_array_first;
  bool last_write = false;
  uchar rd_res_cnt = 0;
  ulong slab_base_addr;
  uint delta;
  bool array_cnt_got = false;
  AddOfflineParsed parsed;

  while (1) {
    bool should_write_add_offline_parsed = false;
    
    bool should_write_array_add_req_info = false;
    ArrayAddReqInfo val_array_add_req_info;
    
    bool read_rd_res;

    if (last_write) {
      last_write = false;
      should_write_add_offline_parsed = true;
      ulong val_res_in_ulong[4];
#pragma unroll
      for (int i = 0; i < 4; i ++) {
	val_res_in_ulong[i] =
	  (((ulong)val_res_in_uchar[0 + (i << 3)]) << 56) |
	  (((ulong)val_res_in_uchar[1 + (i << 3)]) << 48) |
	  (((ulong)val_res_in_uchar[2 + (i << 3)]) << 40) |
	  (((ulong)val_res_in_uchar[3 + (i << 3)]) << 32) |
	  (((ulong)val_res_in_uchar[4 + (i << 3)]) << 24) |
	  (((ulong)val_res_in_uchar[5 + (i << 3)]) << 16) |
	  (((ulong)val_res_in_uchar[6 + (i << 3)]) << 8) |
	  (((ulong)val_res_in_uchar[7 + (i << 3)]) << 0);
      }
      parsed.val.x = val_res_in_ulong[0];
      parsed.val.y = val_res_in_ulong[1];
      parsed.val.z = val_res_in_ulong[2];
      parsed.val.w = val_res_in_ulong[3];
    }
    
    ulong4 data = read_channel_nb_altera(slab_fetcher_add_offline_dma_rd_res, &read_rd_res);
    
    if (read_rd_res) {
      bool first_res = false;
      if (!inflight_rd_res_size) {
	bool dummy;
	AddOfflineType addOfflineType;
	addOfflineType = read_channel_nb_altera(slab_fetcher_add_offline_dma_rd_res_size_with_net_meta, &dummy);
	assert(dummy);
	inflight_rd_res_size = addOfflineType.size;
	net_meta = addOfflineType.net_meta;
	is_array_first = addOfflineType.is_array_first;
	is_array = addOfflineType.is_array;
	slab_base_addr = addOfflineType.address;
	delta = addOfflineType.delta;
	first_res = true;
	rd_res_cnt = 0;
      }
      else {
	rd_res_cnt ++;
      }
      
      uchar data_in_uchar[32];
#pragma unroll      
      for (int i = 0; i < 8; i ++) {
	// convert data in uchar array
	data_in_uchar[i] = ((ulong)(data.x << (i << 3))) >> 56;
	data_in_uchar[i + 8] = ((ulong)(data.y << (i << 3))) >> 56;
	data_in_uchar[i + 16] = ((ulong)(data.z << (i << 3))) >> 56;
	data_in_uchar[i + 24] = ((ulong)(data.w << (i << 3))) >> 56;
      }

      if (first_res) {
	// extract metadata in first packet
	// 2B = 5b(key_size) + 11b(val_size)
	key_size = data_in_uchar[0] >> 3;
	val_size_left = val_size = ((((ushort)data_in_uchar[0] & 7)) << 8) | (data_in_uchar[1]);
	// can add the whole key within first packet, key_size <= 30
#pragma unroll
	for (int i = 0; i < 32; i ++) {
	  if (i < key_size && (2 + i) < 32) {
	    key_res_in_uchar[i] = data_in_uchar[2 + i];
	  }
	  else {
	    key_res_in_uchar[i] = 0;
	  }
	}

	ulong key_res_in_ulong[4];
#pragma unroll	  
	for (int i = 0; i < 4; i ++) {
	  key_res_in_ulong[i] =
	    (((ulong)key_res_in_uchar[0 + (i << 3)]) << 56) |
	    (((ulong)key_res_in_uchar[1 + (i << 3)]) << 48) |
	    (((ulong)key_res_in_uchar[2 + (i << 3)]) << 40) |
	    (((ulong)key_res_in_uchar[3 + (i << 3)]) << 32) |
	    (((ulong)key_res_in_uchar[4 + (i << 3)]) << 24) |
	    (((ulong)key_res_in_uchar[5 + (i << 3)]) << 16) |
	    (((ulong)key_res_in_uchar[6 + (i << 3)]) << 8) |
	    (((ulong)key_res_in_uchar[7 + (i << 3)]) << 0);
	}
	parsed.key.x = key.x = key_res_in_ulong[0];
	parsed.key.y = key.y = key_res_in_ulong[1];
	parsed.key.z = key.z = key_res_in_ulong[2];
	parsed.key.w = key.w = key_res_in_ulong[3];
	parsed.delta = delta;
	parsed.is_array_first = is_array_first;
	parsed.is_array = is_array;
	parsed.net_meta = net_meta;
	parsed.base_addr = slab_base_addr;
	parsed.key_size = key_size;
	parsed.val_size = val_size;
	
	// the lefting part belongs to value field
	val_res_idx = 30 - key_size;
#pragma unroll	  
	for (int i = 0; i < 32; i ++) {
	  if (i < val_res_idx && 2 + key_size + i < 32) {
	    val_res_in_uchar[i] = data_in_uchar[2 + key_size + i];
	  }
	  else {
	    val_res_in_uchar[i] = 0;
	  }
	}

	if (is_array_first) {
	  if (val_res_idx >= 2) {
	    // got array size 
	    ushort array_cnt =
	      (((ushort)val_res_in_uchar[0]) << 8) |
	      (((ushort)val_res_in_uchar[1]));
	    array_cnt_got = true;
	    ArrayAddReqInfo info;
	    info.net_meta = net_meta;
	    info.key = key;
	    info.cnt = array_cnt;
	    info.key_size = key_size;
	    info.delta = delta;
	    should_write_array_add_req_info = true;
	    val_array_add_req_info = info;
	  }
	}
	
	if (inflight_rd_res_size > 32) {
	  inflight_rd_res_size -= 32;
	}
	else {
	  inflight_rd_res_size = 0;
	  val_res_idx = 0;
	  last_write = true;
	}      	
      }
      else if (val_size_left) {
	// not the first packet => no metadata
	// all the contents of this packet belong to val field	
	
#define unroll_bs(idx) \
	if (val_res_idx == idx && i >= idx) { \
	  val_res_in_uchar[i] = data_in_uchar[i - idx]; \
	}
	
#pragma unroll
	for (int i = 0; i < 32; i ++) {
	  UNROLL_0_to_31;
	}
#undef unroll_bs	

	if (is_array_first) {
	  if (!array_cnt_got) {
	    // got array size
	    array_cnt_got = true;
	    ushort array_cnt =
	      (((ushort)val_res_in_uchar[0]) << 8) |
	      (((ushort)val_res_in_uchar[1]));
	    array_cnt_got = true;
	    ArrayAddReqInfo info;
	    info.net_meta = net_meta;
	    info.key = key;
	    info.cnt = array_cnt;
	    info.key_size = key_size;
	    should_write_array_add_req_info = true;
	    val_array_add_req_info = info;
	  }
	}
	
	ulong val_res_in_ulong[4];
#pragma unroll
	for (int i = 0; i < 4; i ++) {
	  val_res_in_ulong[i] =
	    (((ulong)val_res_in_uchar[0 + (i << 3)]) << 56) |
	    (((ulong)val_res_in_uchar[1 + (i << 3)]) << 48) |
	    (((ulong)val_res_in_uchar[2 + (i << 3)]) << 40) |
	    (((ulong)val_res_in_uchar[3 + (i << 3)]) << 32) |
	    (((ulong)val_res_in_uchar[4 + (i << 3)]) << 24) |
	    (((ulong)val_res_in_uchar[5 + (i << 3)]) << 16) |
	    (((ulong)val_res_in_uchar[6 + (i << 3)]) << 8) |
	    (((ulong)val_res_in_uchar[7 + (i << 3)]) << 0);
	}
	parsed.val.x = val_res_in_ulong[0];
	parsed.val.y = val_res_in_ulong[1];
	parsed.val.z = val_res_in_ulong[2];
	parsed.val.w = val_res_in_ulong[3];
	should_write_add_offline_parsed = true;
	
#pragma unroll
	for (int i = 0; i < 32; i ++) {
	  if (i < val_res_idx) {
	    val_res_in_uchar[i] = data_in_uchar[32 - val_res_idx + i];
	  }
	  else {
	    val_res_in_uchar[i] = 0;
	  }
	}

	if (val_size_left > 32) {
	  val_size_left -= 32;
	}
	else {
	  val_size_left = 0;
	}
	
	if (inflight_rd_res_size > 32) {
	  inflight_rd_res_size -= 32;
	}
	else {
	  inflight_rd_res_size = 0;
	  val_res_idx = 0;
	  last_write = val_size_left;
	}
      }      
    }

    if (should_write_array_add_req_info) {
      bool dummy = write_channel_nb_altera(array_add_req_info, val_array_add_req_info);
      assert(dummy);
    }

    if (should_write_add_offline_parsed) {
      bool dummy = write_channel_nb_altera(add_offline_parsed, parsed);
      assert(dummy);
    }
    
  }
}
