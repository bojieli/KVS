_CL_VOID
hashtable_put_comparator_pipeline_two() {
  ushort inflight_val_size_left = 0;
  bool last_finished = true;

  ulong2 init_data = read_channel_altera(init_hashtable_put_comparator_pipeline_one);
  ulong slab_start_addr = init_data.x;
  ulong line_start_addr = init_data.y;

  bool should_forwarding_to_return_put_req = false;
  bool should_forwarding_to_put_offline_handler = false;
  bool should_forwarding_to_put_newline_handler = false;

  while (1) {
    bool should_write_put_inline_dma_wr_req_double = false;
    bool should_write_return_put_req = false;
    DMA_WriteReq_Compressed_Double val_write_put_inline_dma_wr_req_double;
    PutReq val_write_return_put_req;
    bool should_write_put_inline_res = false;
    PutRes val_write_put_inline_res;
    bool should_write_put_offline_handler = false;
    PutOfflineType val_write_put_offline_handler;
    bool should_write_put_newline_handler = false;
    PutNewlineType val_write_put_newline_handler;
    
    PutReq req;
    uchar line_in_uchar[64];

    ulong8 line;

    PutComparatorIntermInfo val_put_comparator_interm_info;
    bool read_put_comparator_interm_info = false;

    val_put_comparator_interm_info = read_channel_nb_altera(put_comparator_interm_info, &read_put_comparator_interm_info);
    
    if (read_put_comparator_interm_info) {
      req = val_put_comparator_interm_info.req;
      
      if (last_finished) {
	last_finished = false;
	ulong4 key = req.key;
	inflight_val_size_left = req.val_size;

#pragma unroll
	for (int i = 0; i < 64; i ++) {
	  line_in_uchar[i] = val_put_comparator_interm_info.line[i];
	}

	ulong tmp[8];
#pragma unroll
	for (int i = 0; i < 8; i ++) {
	  tmp[i] =
	    (ulong)line_in_uchar[(i << 3) + 0] << (7 << 3) |
	    (ulong)line_in_uchar[(i << 3) + 1] << (6 << 3) |
	    (ulong)line_in_uchar[(i << 3) + 2] << (5 << 3) |
	    (ulong)line_in_uchar[(i << 3) + 3] << (4 << 3) |
	    (ulong)line_in_uchar[(i << 3) + 4] << (3 << 3) |
	    (ulong)line_in_uchar[(i << 3) + 5] << (2 << 3) |
	    (ulong)line_in_uchar[(i << 3) + 6] << (1 << 3) |
	    (ulong)line_in_uchar[(i << 3) + 7] << (0 << 3);	 
	}

	line.lo.x = tmp[0];
	line.lo.y = tmp[1];
	line.lo.z = tmp[2];
	line.lo.w = tmp[3];
	line.hi.x = tmp[4];
	line.hi.y = tmp[5];
	line.hi.z = tmp[6];
	line.hi.w = tmp[7];	

	if (val_put_comparator_interm_info.line_updated) {
	  DMA_WriteReq_Compressed wr_req_compressed;
	  if (!req.has_last) {
	    wr_req_compressed.address = ((req.hash1) << 6) + line_start_addr + 32;
	  }
	  else {
	    wr_req_compressed.address = ((req.hash1) << 5) + slab_start_addr + 32;
	  }
	  wr_req_compressed.size = 32;
	  ulong tmp[4];
#pragma unroll	      	      
	  for (int i = 0; i < 4; i ++) {
	    tmp[i] =
	      (((ulong)line_in_uchar[32 + 0 + (i << 3)]) << 56) |
	      (((ulong)line_in_uchar[32 + 1 + (i << 3)]) << 48) |
	      (((ulong)line_in_uchar[32 + 2 + (i << 3)]) << 40) |
	      (((ulong)line_in_uchar[32 + 3 + (i << 3)]) << 32) |
	      (((ulong)line_in_uchar[32 + 4 + (i << 3)]) << 24) |
	      (((ulong)line_in_uchar[32 + 5 + (i << 3)]) << 16) |
	      (((ulong)line_in_uchar[32 + 6 + (i << 3)]) << 8) |
	      (((ulong)line_in_uchar[32 + 7 + (i << 3)]) << 0);
	  }
	  wr_req_compressed.data.x = tmp[0];
	  wr_req_compressed.data.y = tmp[1];
	  wr_req_compressed.data.z = tmp[2];
	  wr_req_compressed.data.w = tmp[3];
		
	  val_write_put_inline_dma_wr_req_double.x = wr_req_compressed;
	  val_write_put_inline_dma_wr_req_double.valid2 = false;
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

	if (!req.ins_fin) {
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

	  bool is_empty[10];
#pragma unroll
	  for (int i = 0; i < 10; i ++) {
	    if (i == 0) {
	      is_empty[i] = (line_5B_metadata[i] == 0);
	    }
	    else {
	      is_empty[i] = ((line_5B_metadata[i] == 0) && ((line_5B_metadata[i - 1] >> 3) != 3));
	    }
	  }

	  bool found = false;
	  if (req.key_size + req.val_size <= 15) {	    
	    // inline insert
	    uchar put_idx;
	    
	    if (req.key_size + req.val_size <= 10) {
	      // need 5B * 2
#pragma unroll
	      for (int i = 0; i < 9; i ++) {
		if (is_empty[i] && is_empty[i + 1]) {
		  put_idx = i;
		  found = true;
		}
	      }	      
	    }
	    else {
	      // need 5B * 3
#pragma unroll
	      for (int i = 0; i < 8; i ++) {
		if (is_empty[i] && is_empty[i + 1] & is_empty[i + 2]) {
		  put_idx = i;
		  found = true;
		}
	      }	      
	    }

	    if (found) {	      
	      uchar line_in_1bit[512];
	      // modify data in 1bit granularity
#pragma unroll
	      for (int i = 0; i < 512; i ++) {
		line_in_1bit[i] = ((line_in_uchar[i >> 3]) >> (7 - (i & 7))) & 1;
	      }

	      // modify metadata
#pragma unroll	      
	      for (int i = 400; i < 512; i ++) {
		if (i == 400 + 5 * put_idx) {
		  line_in_1bit[i] = 1 & 1;
		}
		else if (i == 400 + 5 * put_idx + 1) {
		  line_in_1bit[i] = 1 & 1;
		}
		else if (i == 400 + 5 * put_idx + req.key_size - 1) {
		  line_in_1bit[i] = 1 & 1;		
		}
		else if (i == 400 + 5 * put_idx + req.key_size + req.val_size - 1) {
		  line_in_1bit[i] = 1 & 1; 
		}	      
	      }
	    
	      uchar keyval_in_1bit[120];
#pragma unroll
	      for (int i = 0; i < 64; i ++) {
		keyval_in_1bit[i] = (req.key.x >> (63 - i)) & 1;
		if (i + 64 < 120) {
		  keyval_in_1bit[i + 64] = (req.key.y >> (63 - i)) & 1;
		}
	      }

#pragma unroll
	      for (int i = 0; i < 64; i ++) {
		if (req.key_size == 3) {
		  if (i + ((3) << 3) < 120) {
		    keyval_in_1bit[i + ((3) << 3)] = (req.val.x >> (63 - i)) & 1;
		  }
		  if (i + ((3) << 3) + 64 < 120) {
		    keyval_in_1bit[i + ((3) << 3) + 64] = (req.val.y >> (63 - i)) & 1;
		  }
		}
		else if (req.key_size == 4) {
		  if (i + ((4) << 3) < 120) {
		    keyval_in_1bit[i + ((4) << 3)] = (req.val.x >> (63 - i)) & 1;
		  }
		  if (i + ((4) << 3) + 64 < 120) {
		    keyval_in_1bit[i + ((4) << 3) + 64] = (req.val.y >> (63 - i)) & 1;

		  }
		}
		else if (req.key_size == 5) {
		  if (i + ((5) << 3) < 120) {
		    keyval_in_1bit[i + ((5) << 3)] = (req.val.x >> (63 - i)) & 1;
		  }
		  if (i + ((5) << 3) + 64 < 120) {		  
		    keyval_in_1bit[i + ((5) << 3) + 64] = (req.val.y >> (63 - i)) & 1;
		  }
		}
		else if (req.key_size == 6) {
		  if (i + ((6) << 3) < 120) {		  
		    keyval_in_1bit[i + ((6) << 3)] = (req.val.x >> (63 - i)) & 1;
		  }
		  if (i + ((6) << 3) + 64 < 120) {		  		  
		    keyval_in_1bit[i + ((6) << 3) + 64] = (req.val.y >> (63 - i)) & 1;
		  }
		}
		else if (req.key_size == 7) {
		  if (i + ((7) << 3) < 120) {		  
		    keyval_in_1bit[i + ((7) << 3)] = (req.val.x >> (63 - i)) & 1;
		  }
		  if (i + ((7) << 3) + 64 < 120) {		  		  
		    keyval_in_1bit[i + ((7) << 3) + 64] = (req.val.y >> (63 - i)) & 1;
		  }
		}
		else if (req.key_size == 8) {
		  if (i + ((8) << 3) < 120) {
		    keyval_in_1bit[i + ((8) << 3)] = (req.val.x >> (63 - i)) & 1;
		  }
		  if (i + ((8) << 3) + 64 < 120) {
		    keyval_in_1bit[i + ((8) << 3) + 64] = (req.val.y >> (63 - i)) & 1;
		  }
		}
		else if (req.key_size == 9) {
		  if (i + ((9) << 3) < 120) {
		    keyval_in_1bit[i + ((9) << 3)] = (req.val.x >> (63 - i)) & 1;
		  }
		  if (i + ((9) << 3) + 64 < 120) {
		    keyval_in_1bit[i + ((9) << 3) + 64] = (req.val.y >> (63 - i)) & 1;
		  }
		}
		else if (req.key_size == 10) {
		  if (i + ((10) << 3) < 120) {		  
		    keyval_in_1bit[i + ((10) << 3)] = (req.val.x >> (63 - i)) & 1;
		  }
		  if (i + ((10) << 3) + 64 < 120) {
		    keyval_in_1bit[i + ((10) << 3) + 64] = (req.val.y >> (63 - i)) & 1;
		  }
		}
		else if (req.key_size == 11) {
		  if (i + ((11) << 3) < 120) {		  
		    keyval_in_1bit[i + ((11) << 3)] = (req.val.x >> (63 - i)) & 1;
		  }
		  if (i + ((11) << 3) + 64 < 120) {		  		  
		    keyval_in_1bit[i + ((11) << 3) + 64] = (req.val.y >> (63 - i)) & 1;
		  }
		}
		else if (req.key_size == 12) {
		  if (i + ((12) << 3) < 120) {		  		  
		    keyval_in_1bit[i + ((12) << 3)] = (req.val.x >> (63 - i)) & 1;
		  }
		  if (i + ((12) << 3) + 64 < 120) {		  		  
		    keyval_in_1bit[i + ((12) << 3) + 64] = (req.val.y >> (63 - i)) & 1;
		  }
		}		
	      }

	      // modify data
#pragma unroll
	      for (int i = 0; i < 120; i ++) {
		if (i < ((req.key_size + req.val_size) << 3)) {
		  if (put_idx == 0) {
		    line_in_1bit[i + 40 * (0)] = keyval_in_1bit[i] & 1;
		  }
		  else if (put_idx == 1) {
		    line_in_1bit[i + 40 * (1)] = keyval_in_1bit[i] & 1;
		  }
		  else if (put_idx == 2) {
		    line_in_1bit[i + 40 * (2)] = keyval_in_1bit[i] & 1;
		  }
		  else if (put_idx == 3) {
		    line_in_1bit[i + 40 * (3)] = keyval_in_1bit[i] & 1;
		  }
		  else if (put_idx == 4) {
		    line_in_1bit[i + 40 * (4)] = keyval_in_1bit[i] & 1;
		  }
		  else if (put_idx == 5) {
		    line_in_1bit[i + 40 * (5)] = keyval_in_1bit[i] & 1;
		  }
		  else if (put_idx == 6) {
		    line_in_1bit[i + 40 * (6)] = keyval_in_1bit[i] & 1;
		  }
		  else if (put_idx == 7) {
		    line_in_1bit[i + 40 * (7)] = keyval_in_1bit[i] & 1;
		  }
		  else if (put_idx == 8) {
		    line_in_1bit[i + 40 * (8)] = keyval_in_1bit[i] & 1;
		  }						
		}
	      }

	      uchar data[64];
#pragma unroll
	      for (int i = 0; i < 64; i ++) {
		data[i] =
		  (line_in_1bit[(i << 3)] << 7) |
		  (line_in_1bit[(i << 3) + 1] << 6) |
		  (line_in_1bit[(i << 3) + 2] << 5) |
		  (line_in_1bit[(i << 3) + 3] << 4) |
		  (line_in_1bit[(i << 3) + 4] << 3) |
		  (line_in_1bit[(i << 3) + 5] << 2) |
		  (line_in_1bit[(i << 3) + 6] << 1) |
		  (line_in_1bit[(i << 3) + 7] << 0);
	      }

	      req.ins_fin = true;
	      should_write_put_inline_dma_wr_req_double = true;
	      DMA_WriteReq_Compressed wr_req_compressed;
	      // update metadata && data
	      if (put_idx <= 6) {
		// need double dma writes
		if (!req.has_last) {
		  wr_req_compressed.address = ((req.hash1) << 6) + line_start_addr;
		}
		else {
		  wr_req_compressed.address = ((req.hash1) << 5) + slab_start_addr; 
		}
		wr_req_compressed.size = 64;
		ulong tmp[8];
#pragma unroll	      
		for (int i = 0; i < 8; i ++) {
		  tmp[i] =
		    (((ulong)data[0 + (i << 3)]) << 56) |
		    (((ulong)data[1 + (i << 3)]) << 48) |
		    (((ulong)data[2 + (i << 3)]) << 40) |
		    (((ulong)data[3 + (i << 3)]) << 32) |
		    (((ulong)data[4 + (i << 3)]) << 24) |
		    (((ulong)data[5 + (i << 3)]) << 16) |
		    (((ulong)data[6 + (i << 3)]) << 8) |
		    (((ulong)data[7 + (i << 3)]) << 0);
		}
		wr_req_compressed.data.x = tmp[0];
		wr_req_compressed.data.y = tmp[1];
		wr_req_compressed.data.z = tmp[2];
		wr_req_compressed.data.w = tmp[3];		
		val_write_put_inline_dma_wr_req_double.x = wr_req_compressed;

		wr_req_compressed.address = 0;
		wr_req_compressed.size = 0;
		wr_req_compressed.data.x = tmp[4];
		wr_req_compressed.data.y = tmp[5];
		wr_req_compressed.data.z = tmp[6];
		wr_req_compressed.data.w = tmp[7];
		val_write_put_inline_dma_wr_req_double.y = wr_req_compressed;
		val_write_put_inline_dma_wr_req_double.valid2 = true;
	      }
	      else {
		// can coalesce into single dma write
		if (!req.has_last) {
		  wr_req_compressed.address = ((req.hash1) << 6) + line_start_addr + 32;
		}
		else {
		  wr_req_compressed.address = ((req.hash1) << 5) + slab_start_addr + 32;
		}
		wr_req_compressed.size = 32;
		ulong tmp[4];
#pragma unroll	      	      
		for (int i = 0; i < 4; i ++) {
		  tmp[i] =
		    (((ulong)data[32 + 0 + (i << 3)]) << 56) |
		    (((ulong)data[32 + 1 + (i << 3)]) << 48) |
		    (((ulong)data[32 + 2 + (i << 3)]) << 40) |
		    (((ulong)data[32 + 3 + (i << 3)]) << 32) |
		    (((ulong)data[32 + 4 + (i << 3)]) << 24) |
		    (((ulong)data[32 + 5 + (i << 3)]) << 16) |
		    (((ulong)data[32 + 6 + (i << 3)]) << 8) |
		    (((ulong)data[32 + 7 + (i << 3)]) << 0);
		}
		wr_req_compressed.data.x = tmp[0];
		wr_req_compressed.data.y = tmp[1];
		wr_req_compressed.data.z = tmp[2];
		wr_req_compressed.data.w = tmp[3];
		
		val_write_put_inline_dma_wr_req_double.x = wr_req_compressed;
		val_write_put_inline_dma_wr_req_double.valid2 = false;
	      }

	      should_write_put_inline_res = true;
	      val_write_put_inline_res.found = true;
	      val_write_put_inline_res.net_meta = req.net_meta;
	      val_write_put_inline_res.key_size = req.key_size;
	      val_write_put_inline_res.key = req.key;
	    }
	  }
	  else {
	    // offline insert
	    uchar put_idx;
#pragma unroll
	    for (int i = 0; i < 10; i ++) {
	      if (is_empty[i]) {
		put_idx = i;
		found = true;
	      }
	    }

	    if (found) {
	      // modify data field and metadata field
	      // specifically, 
	      // 1. modify the metadata of line
	      // 2. apply a slab whose size = 2 + key_size + val_size
	      // 3. write {key_size, val_size, key, val} into that slab
	      req.ins_fin = true;
	      SlabRequest slabRequest;
	      slabRequest.slab_size = 2 + req.key_size + req.val_size;
	      bool dummy;
	      dummy = write_channel_nb_altera(hashtable_put_offline_slab_req, slabRequest);
	      assert(dummy);
	      should_write_put_offline_handler = true;
	      should_forwarding_to_put_offline_handler = true;
	      val_write_put_offline_handler.req = req;
	      val_write_put_offline_handler.line = line;
	      val_write_put_offline_handler.put_idx = put_idx;
	    }
	  }
	} // end: if (!ins_fin)

	if (!(req.ins_fin && req.del_fin)) {
	  if ((line_in_uchar[63] & 1) == 1) {
	    // actually save the addr of the nexting chain here, not to update hash1
	    req.hash1 =
	      (line_in_uchar[60] << 24) |
	      (line_in_uchar[61] << 16) |
	      (line_in_uchar[62] << 8) |
	      line_in_uchar[63];
	    req.hash1 >>= 2; // remove the last 2b(valid bit + reserved bit)
	    should_write_return_put_req = true;
	    val_write_return_put_req = req;
	    should_forwarding_to_return_put_req = true;
	  }
	  else if (!req.ins_fin) {
	    // cannot find, apply new line
	    SlabRequest slabRequest;
	    slabRequest.slab_size = 64;
	    bool dummy;
	    dummy = write_channel_nb_altera(hashtable_put_newline_slab_req, slabRequest);
	    assert(dummy);
	    should_write_put_newline_handler = true;
	    should_forwarding_to_put_newline_handler = true;
	    val_write_put_newline_handler.req = req;
	    val_write_put_newline_handler.half_line = line.hi;
	  }
	}
      }
      else {
	// forwarding show_ahead_fetching_put_req
	assert(
	       should_forwarding_to_return_put_req ||
	       should_forwarding_to_put_offline_handler ||
	       should_forwarding_to_put_newline_handler
	       );
	if (should_forwarding_to_return_put_req) {
	  should_write_return_put_req = true;
	  val_write_return_put_req = req;
	}
	else if (should_forwarding_to_put_offline_handler) {
	  should_write_put_offline_handler = true;
	  val_write_put_offline_handler.req = req;
	}
	else if (should_forwarding_to_put_newline_handler) {
	  should_write_put_newline_handler = true;
	  val_write_put_newline_handler.req = req;
	}
      }
      
      if (inflight_val_size_left > 32) {
	inflight_val_size_left -= 32;
      }
      else {
	inflight_val_size_left = 0;
	last_finished = true;
	should_forwarding_to_return_put_req = false;
	should_forwarding_to_put_offline_handler = false;
	should_forwarding_to_put_newline_handler = false;
      }
      
      if (should_write_return_put_req) {
	bool dummy;
	dummy = write_channel_nb_altera(return_put_req, val_write_return_put_req);
	assert(dummy);
      }

      if (should_write_put_offline_handler) {
	bool dummy;
	dummy = write_channel_nb_altera(put_offline_handler, val_write_put_offline_handler);
	assert(dummy);
      }
   
      if (should_write_put_inline_res) {
	bool dummy;
	dummy = write_channel_nb_altera(put_inline_res, val_write_put_inline_res);
	assert(dummy);
      }
      
      if (should_write_put_inline_dma_wr_req_double) {
	bool dummy;
	dummy = write_channel_nb_altera(hashtable_put_inline_dma_wr_req_double, val_write_put_inline_dma_wr_req_double);
	assert(dummy);
      }

      if (should_write_put_newline_handler) {
	bool dummy;
	dummy = write_channel_nb_altera(put_newline_handler, val_write_put_newline_handler);
	assert(dummy);
      }

    }
      
  }
}
