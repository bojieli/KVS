// initially copy from get/comparator.cpp and do further modification
_CL_VOID
hashtable_add_comparator() {
  ulong2 init_data = read_channel_altera(init_hashtable_add_comparator);
  ulong slab_start_addr = init_data.x;
  ulong line_start_addr = init_data.y;

  while (1) {
    ulong8 line;
    bool read_line;
    AddReq req;

    bool should_write_add_inline_res = false;
    AddRes val_write_add_inline_res;

    bool should_write_hashtable_add_inline_update_line_dma_wr_req_double = false;
    DMA_WriteReq_Compressed_Double val_hashtable_add_inline_update_line_dma_wr_req_double;

    bool read_line_fetcher_add_dma_rd_res;
    line = read_channel_nb_altera(line_fetcher_add_dma_rd_res, &read_line_fetcher_add_dma_rd_res);

    if (read_line_fetcher_add_dma_rd_res) {
      bool dummy;
      req = read_channel_nb_altera(fetching_add_req, &dummy);
      assert(dummy);	

      ulong4 key = req.key;
      uchar line_in_uchar[64];
      ulong tmp[8] =
	{
	  line.lo.x, line.lo.y, line.lo.z, line.lo.w,
	  line.hi.x, line.hi.y, line.hi.z, line.hi.w
	};
      
#pragma unroll
      for (int i = 0; i < 64; i ++) {
	line_in_uchar[i] = ((ulong)(tmp[i >> 3] << ((i & 7) << 3))) >> (64 - 8);
      }

      uchar line_data_in_5B[10];
      uchar line_5B_metadata[10];
      line_5B_metadata[0] = line_in_uchar[50] >> 3;
      line_5B_metadata[1] = ((line_in_uchar[50] & 7) << 2) | (line_in_uchar[51] >> 6);
      line_5B_metadata[2] = ((uchar)(line_in_uchar[51] << 2)) >> 3;
      line_5B_metadata[3] = ((line_in_uchar[51] & 1) << 4) | (line_in_uchar[52] >> 4);
      line_5B_metadata[4] = ((line_in_uchar[52] & 15) << 1) | (line_in_uchar[53] >> 7);
      line_5B_metadata[5] = (uchar)(line_in_uchar[53] << 1) >> 3;
      line_5B_metadata[6] = ((line_in_uchar[53] & 3) << 3) | (line_in_uchar[54] >> 5);
      line_5B_metadata[7] = line_in_uchar[54] & 31;
      line_5B_metadata[8] = line_in_uchar[55] >> 3;
      line_5B_metadata[9] = ((line_in_uchar[55] & 7) << 2) | (line_in_uchar[56] >> 6);

      bool offline_found = false;
      uchar offline_found_slab_type;
      ulong offline_found_val_addr;
      // do offline comparasion

#pragma unroll
      for (int i = 0; i < 10; i ++) {
	if (line_5B_metadata[i] == 31) { // 5'b11111
	  ushort line_hash2 = (((ushort)line_in_uchar[5 * i]) << 2) | (((ushort)line_in_uchar[5 * i + 1]) >> 6);
	  if (line_hash2 == req.hash2) {
	    offline_found = true;
	    offline_found_val_addr =
	      (((ulong)(((uchar)(line_in_uchar[5 * i + 1] << 2)) >> 2)) << 24) |
	      (((ulong)(line_in_uchar[5 * i + 2])) << 16) |
	      (((ulong)(line_in_uchar[5 * i + 3])) << 8) |
	      ((ulong)(line_in_uchar[5 * i + 4]));

	    offline_found_val_addr <<= 5;
	    offline_found_slab_type =
	      (i == 0) ? (((uchar)(line_in_uchar[56] << 2)) >> 5) :
	      (i == 1) ? (((uchar)(line_in_uchar[56] << 5)) >> 5) :
	      (i == 2) ? ((line_in_uchar[57]) >> 5) :
	      (i == 3) ? (((uchar)(line_in_uchar[57] << 3)) >> 5) :
	      (i == 4) ? ((((uchar)(line_in_uchar[57] << 6)) >> 6) << 1) | (line_in_uchar[58] >> 7) :
	      (i == 5) ? (((uchar)(line_in_uchar[58] << 1)) >> 5) :
	      (i == 6) ? (((uchar)(line_in_uchar[58] << 4)) >> 5) :
	      (i == 7) ? ((((uchar)(line_in_uchar[58] << 7)) >> 7) << 2) | (line_in_uchar[59] >> 6):
	      (i == 8) ? (((uchar)(line_in_uchar[59] << 2)) >> 5):
	      (((uchar)(line_in_uchar[59] << 5)) >> 5);
	  }
	}
      }

      bool inline_found_slots[10];
#pragma unroll
      for (int i = 0; i < 10; i ++) {
	inline_found_slots[i] = false;
      }
      
      ulong2 inline_found_val_slots[10];
      uchar inline_found_val_start_idx_slots[10];
      uchar inline_found_val_end_idx_slots[10];
      
      // do inline comparasion
      if (req.key_size <= 12) {
	// otherwise cannot been inlined, since key_size + val_size <= 15 when inlined && key_size >= 3 && val_size >= 3
#pragma unroll
	for (int i = 0; i < 10; i ++) {
	  bool is_true[15];
	  is_true[0] = (line_5B_metadata[i] >> 4) & 1;
	  is_true[1] = (((uchar)(line_5B_metadata[i] << 1)) >> 4) & 1;
	  is_true[2] = (((uchar)(line_5B_metadata[i] << 2)) >> 4) & 1;
	  is_true[3] = (((uchar)(line_5B_metadata[i] << 3)) >> 4) & 1;
	  is_true[4] = (((uchar)(line_5B_metadata[i] << 4)) >> 4) & 1;
	  
	  is_true[5] = (line_5B_metadata[i + 1] >> 4) & 1;
	  is_true[6] = (((uchar)(line_5B_metadata[i + 1] << 1)) >> 4) & 1;
	  is_true[7] = (((uchar)(line_5B_metadata[i + 1] << 2)) >> 4) & 1;
	  is_true[8] = (((uchar)(line_5B_metadata[i + 1] << 3)) >> 4) & 1;
	  is_true[9] = (((uchar)(line_5B_metadata[i + 1] << 4)) >> 4) & 1;

	  is_true[10] = (line_5B_metadata[i + 2] >> 4) & 1;
	  is_true[11] = (((uchar)(line_5B_metadata[i + 2] << 1)) >> 4) & 1;
	  is_true[12] = (((uchar)(line_5B_metadata[i + 2] << 2)) >> 4) & 1;
	  is_true[13] = (((uchar)(line_5B_metadata[i + 2] << 3)) >> 4) & 1;
	  is_true[14] = (((uchar)(line_5B_metadata[i + 2] << 4)) >> 4) & 1;

	  if (is_true[0] && is_true[1]) {
	    inline_found_slots[i] = true;
#pragma unroll
	    for (int j = 0; j < 12; j ++) {
	      if (j + 2 < req.key_size - 1) {
		if (is_true[j + 2]) {
		  inline_found_slots[i] = false;
		}
	      }
	    }
	    if (inline_found_slots[i]) {
	      if (!is_true[req.key_size - 1]) {
		inline_found_slots[i] = false;
	      }
	      else {
		uchar key_in_uchar[12];
		key_in_uchar[0] = req.key.x >> 56;
		key_in_uchar[1] = ((ulong)(req.key.x << 8)) >> 56;
		key_in_uchar[2] = ((ulong)(req.key.x << 16)) >> 56;
		key_in_uchar[3] = ((ulong)(req.key.x << 24)) >> 56;
		key_in_uchar[4] = ((ulong)(req.key.x << 32)) >> 56;
		key_in_uchar[5] = ((ulong)(req.key.x << 40)) >> 56;
		key_in_uchar[6] = ((ulong)(req.key.x << 48)) >> 56;
		key_in_uchar[7] = ((ulong)(req.key.x << 56)) >> 56;
		
		key_in_uchar[8] = req.key.y >> 56;
		key_in_uchar[9] = ((ulong)(req.key.y << 8)) >> 56;
		key_in_uchar[10] = ((ulong)(req.key.y << 16)) >> 56;
		key_in_uchar[11] = ((ulong)(req.key.y << 24)) >> 56;
		
		uchar line_key_in_uchar[12];
#pragma unroll		
		for (int j = 0; j < 12; j ++) {
		  if (j < req.key_size) {
		    line_key_in_uchar[j] = line_in_uchar[5 * i + j];
		  }
		  else {
		    line_key_in_uchar[j] = 0;
		  }
		}

#pragma unroll
		for (int j = 0; j < 12; j ++) {
		  if (key_in_uchar[j] != line_key_in_uchar[j]) {
		    inline_found_slots[i] = false;
		  }
		}		

		if (inline_found_slots[i]) {		  
		  ushort val_start_idx = req.key_size;
		  ushort val_end_idx;

		  val_end_idx =
		    (5 > req.key_size && is_true[5]) ? 5 :
		    (6 > req.key_size && is_true[6]) ? 6 :
		    (7 > req.key_size && is_true[7]) ? 7 :
		    (8 > req.key_size && is_true[8]) ? 8 :
		    (9 > req.key_size && is_true[9]) ? 9 :
		    (10 > req.key_size && is_true[10]) ? 10 :
		    (11 > req.key_size && is_true[11]) ? 11 :
		    (12 > req.key_size && is_true[12]) ? 12 :
		    (13 > req.key_size && is_true[13]) ? 13 :
		    14;
		  
		  inline_found_val_start_idx_slots[i] = val_start_idx;
		  inline_found_val_end_idx_slots[i] = val_end_idx;
		  
		  uchar line_val_in_uchar[15];
#pragma unroll		  
		  for (int j = 0; j < 15; j ++) {
		    if (j + val_start_idx <= val_end_idx) {
		      line_val_in_uchar[j] = line_in_uchar[5 * i + j + val_start_idx];
		    }
		    else {
		      line_val_in_uchar[j] = 0;
		    }
		  }

		  inline_found_val_slots[i].x =
		    (((ulong)line_val_in_uchar[0]) << 56) | 
		    (((ulong)line_val_in_uchar[1]) << 48) |
		    (((ulong)line_val_in_uchar[2]) << 40) | 
		    (((ulong)line_val_in_uchar[3]) << 32) |
		    (((ulong)line_val_in_uchar[4]) << 24) | 
		    (((ulong)line_val_in_uchar[5]) << 16) |
		    (((ulong)line_val_in_uchar[6]) << 8) | 
		    ((ulong)line_val_in_uchar[7]);

		  inline_found_val_slots[i].y =
		    (((ulong)line_val_in_uchar[8]) << 56) | 
		    (((ulong)line_val_in_uchar[9]) << 48) |
		    (((ulong)line_val_in_uchar[10]) << 40) | 
		    (((ulong)line_val_in_uchar[11]) << 32);
		}
	      }
	    }
	  }
	}
      }

      bool inline_found = false;
      AddKeyType inline_found_val;
      ushort inline_found_val_start_idx;
      ushort inline_found_val_end_idx;
      ushort inline_found_slots_idx;
#pragma unroll
      for (int i = 0; i < 10; i ++) {
	if (
	    inline_found_slots[i] &&
	    inline_found_val_end_idx_slots[i] - inline_found_val_start_idx_slots[i] + 1 == ADD_KEY_SIZE
	    ) {
	  inline_found_slots_idx = i;
	  inline_found = true;
	  if (ADD_KEY_SIZE == 4) {
	    inline_found_val = (inline_found_val_slots[i].x) >> 32;
	  }
	  inline_found_val_start_idx = inline_found_val_start_idx_slots[i];
	}
      }
      inline_found_val_start_idx += inline_found_slots_idx * 5;
      inline_found_val_end_idx = inline_found_val_start_idx + 3;
      
      if (inline_found) {
	inline_found_val += req.delta;

	uchar new_val_in_uchar[4];
	new_val_in_uchar[0] = (inline_found_val >> (3 << 3)) & 0xFF;
	new_val_in_uchar[1] = (inline_found_val >> (2 << 3)) & 0xFF;
	new_val_in_uchar[2] = (inline_found_val >> (1 << 3)) & 0xFF;
	new_val_in_uchar[3] = (inline_found_val >> (0 << 3)) & 0xFF;

	uchar new_line_in_uchar[64];

#define unroll_sec(idx)							\
	else if (inline_found_val_start_idx == idx && i >= idx && i <= idx + 3) { \
	  new_line_in_uchar[i] = new_val_in_uchar[i - idx];		\
	} 
	
#pragma unroll
	for (int i = 0; i < 64; i ++) {
	  if (inline_found_val_start_idx == 2 && i >= 2 && i <= 2 + 3) { 
	    new_line_in_uchar[i] = new_val_in_uchar[i - 2]; 
	  }
	  UNROLL_3_to_63
	  else {
	    new_line_in_uchar[i] = line_in_uchar[i];
	  }
	}
#undef unroll_sec	
	
	ulong tmp[8];
#pragma unroll
	  for (int i = 0; i < 8; i ++) {
	    tmp[i] =
	      ((ulong)new_line_in_uchar[0 + (i << 3)]) << 56 |
	      ((ulong)new_line_in_uchar[1 + (i << 3)]) << 48 |
	      ((ulong)new_line_in_uchar[2 + (i << 3)]) << 40 |
	      ((ulong)new_line_in_uchar[3 + (i << 3)]) << 32 |
	      ((ulong)new_line_in_uchar[4 + (i << 3)]) << 24 |
	      ((ulong)new_line_in_uchar[5 + (i << 3)]) << 16 |
	      ((ulong)new_line_in_uchar[6 + (i << 3)]) << 8 |
	      ((ulong)new_line_in_uchar[7 + (i << 3)]) << 0;
	  }
	// update inline value
	if (inline_found_val_end_idx < 32) {
	  // do dma write on first half
	  DMA_WriteReq_Compressed wr_req_compressed;
	  wr_req_compressed.size = 32;
	  wr_req_compressed.address = (req.hash1 << 6) + line_start_addr;	  
	  wr_req_compressed.data.x = tmp[0];
	  wr_req_compressed.data.y = tmp[1];
	  wr_req_compressed.data.z = tmp[2];
	  wr_req_compressed.data.w = tmp[3];
	  val_hashtable_add_inline_update_line_dma_wr_req_double.x = wr_req_compressed;
	  val_hashtable_add_inline_update_line_dma_wr_req_double.valid2 = false;
	  should_write_hashtable_add_inline_update_line_dma_wr_req_double = true;
	}
	else if (inline_found_val_start_idx >= 32) {
	  // do dma write on last half
	  DMA_WriteReq_Compressed wr_req_compressed;
	  wr_req_compressed.size = 32;
	  wr_req_compressed.address = (req.hash1 << 6) + line_start_addr + 32;
	  wr_req_compressed.data.x = tmp[4];
	  wr_req_compressed.data.y = tmp[5];
	  wr_req_compressed.data.z = tmp[6];
	  wr_req_compressed.data.w = tmp[7];
	  val_hashtable_add_inline_update_line_dma_wr_req_double.x = wr_req_compressed;
	  val_hashtable_add_inline_update_line_dma_wr_req_double.valid2 = false;
	  should_write_hashtable_add_inline_update_line_dma_wr_req_double = true;
	}
	else {
	  // worst case, this val cross the boundry
	  DMA_WriteReq_Compressed wr_req_compressed;
	  wr_req_compressed.size = 64;
	  wr_req_compressed.address = (req.hash1 << 6) + line_start_addr;
	  wr_req_compressed.data.x = tmp[0];
	  wr_req_compressed.data.y = tmp[1];
	  wr_req_compressed.data.z = tmp[2];
	  wr_req_compressed.data.w = tmp[3];	  
	  val_hashtable_add_inline_update_line_dma_wr_req_double.x = wr_req_compressed;
	  wr_req_compressed.size = wr_req_compressed.address = 0;
	  wr_req_compressed.data.x = tmp[4];
	  wr_req_compressed.data.y = tmp[5];
	  wr_req_compressed.data.z = tmp[6];
	  wr_req_compressed.data.w = tmp[7];
	  val_hashtable_add_inline_update_line_dma_wr_req_double.y = wr_req_compressed;
	  val_hashtable_add_inline_update_line_dma_wr_req_double.valid2 = true;
	  should_write_hashtable_add_inline_update_line_dma_wr_req_double = true;
	}
	// give response
	AddRes res;
	res.found = true;
	res.net_meta = req.net_meta;
	res.key_size = req.key_size;
	res.key = req.key;
	should_write_add_inline_res = true;
	val_write_add_inline_res = res;
      }
      else if (offline_found) {
	DMA_ReadReq rd_req;
	DMA_ReadReq_Compressed rd_req_compressed;
	rd_req.req.address = offline_found_val_addr + slab_start_addr;
	rd_req.req.size = 1 << (offline_found_slab_type + 5);
	AddOfflineType addOfflineType;
	addOfflineType.size = rd_req.req.size;
	addOfflineType.net_meta = req.net_meta;
	addOfflineType.is_array_first = req.is_array_first;
	addOfflineType.is_array = req.is_array;
	addOfflineType.address = rd_req.req.address;
	addOfflineType.delta = req.delta;
	rd_req_compressed.address = rd_req.req.address;
	rd_req_compressed.size = rd_req.req.size;
	bool dummy = write_channel_nb_altera(slab_fetcher_add_offline_dma_rd_req, rd_req_compressed);
	assert(dummy);
	dummy = write_channel_nb_altera(slab_fetcher_add_offline_dma_rd_res_size_with_net_meta, addOfflineType);
	assert(dummy);
      }
      else {
	if ((line_in_uchar[63] & 1) == 1) {
	  // actually save the addr of the nexting chain here, not to update hash1
	  req.hash1 =
	    (line_in_uchar[60] << 24) |
	    (line_in_uchar[61] << 16) |
	    (line_in_uchar[62] << 8) |
	    line_in_uchar[63];
	  req.hash1 >>= 2; // remove the last 2b(valid bit + reserved bit)
	  bool dummy = write_channel_nb_altera(return_add_req, req);
	  assert(dummy);
	}
	else {
	  // cannot not find the accroding key!
	  AddRes res;
	  res.found = false;
	  res.net_meta = req.net_meta;
	  res.key_size = req.key_size;
	  res.key = req.key;
	  should_write_add_inline_res = true;
	  val_write_add_inline_res = res;
	}
      }

      if (should_write_add_inline_res) {
       	bool dummy = write_channel_nb_altera(add_inline_res, val_write_add_inline_res);
	assert(dummy);
      }

      if (should_write_hashtable_add_inline_update_line_dma_wr_req_double) {
	bool dummy = write_channel_nb_altera(hashtable_add_inline_update_line_dma_wr_req_double,
					     val_hashtable_add_inline_update_line_dma_wr_req_double);
	assert(dummy);
      }
      
    }
  }
}
