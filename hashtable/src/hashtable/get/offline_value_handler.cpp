_CL_VOID
hashtable_get_offline_value_handler() {
  ushort inflight_rd_res_size = 0;
  ushort val_size_left = 0;
  uchar val_res_idx = 0;
  uchar key_res_in_uchar[32];
  uchar val_res_in_uchar[32];
  GetRes res;
  ushort key_size;
  ushort val_size;
  uchar net_meta;
  bool is_array_first;
  bool last_write = false;

  while (1) {
    bool read_rd_res;
    bool should_write_get_offline_res = false;

    if (last_write) {
      last_write = false;
      should_write_get_offline_res = true;
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
      res.val.x = val_res_in_ulong[0];
      res.val.y = val_res_in_ulong[1];
      res.val.z = val_res_in_ulong[2];
      res.val.w = val_res_in_ulong[3];
    }
    
    ulong4 data = read_channel_nb_altera(slab_fetcher_get_offline_dma_rd_res, &read_rd_res);
    
    if (read_rd_res) {
      bool first_res = false;
      if (!inflight_rd_res_size) {
	bool dummy;
	GetOfflineType getOfflineType;
	getOfflineType = read_channel_nb_altera(slab_fetcher_get_offline_dma_rd_res_size_with_net_meta, &dummy);
	assert(dummy);
	inflight_rd_res_size = getOfflineType.size;
	net_meta = getOfflineType.net_meta;
	is_array_first = getOfflineType.is_array_first;
	first_res = true;
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

	// can get the whole key within first packet, key_size <= 30

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
	res.key.x = key_res_in_ulong[0];
	res.key.y = key_res_in_ulong[1];
	res.key.z = key_res_in_ulong[2];
	res.key.w = key_res_in_ulong[3];
	
	res.found = true;
	res.net_meta = net_meta;
	res.key_size = key_size;
	res.val_size = val_size;
	res.is_array_first = is_array_first;

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

	if (inflight_rd_res_size > 32) {
	  inflight_rd_res_size -= 32;
	}
	else {
	  inflight_rd_res_size = 0;
	  val_res_idx = 0;
	  last_write = true;
	}      	
      }
      else {
	// not the first packet => no metadata
	// all the contents of this packet belong to val field

#pragma unroll
	for (int i = 0; i < 32; i ++) {
	  if (val_res_idx + i < 32) {
	    val_res_in_uchar[i + val_res_idx] = data_in_uchar[i];
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
	res.val.x = val_res_in_ulong[0];
	res.val.y = val_res_in_ulong[1];
	res.val.z = val_res_in_ulong[2];
	res.val.w = val_res_in_ulong[3];
	should_write_get_offline_res = true;
	  
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
	  if (res.is_array_first) {
	    ArrayGetReqInfo info;
	    info.net_meta = res.net_meta;
	    info.key = res.key;
	    info.key_size = res.key_size;
	    info.cnt = (res.val.x >> 48) - 1;
	    if (info.cnt) {
	      bool dummy = write_channel_nb_altera(array_req_info, info);
	      assert(dummy);
	    }
	  }
	}	
      }      
    }
    
    if (should_write_get_offline_res) {
      bool dummy = write_channel_nb_altera(get_offline_res, res);
      assert(dummy);
    }
  }
}

