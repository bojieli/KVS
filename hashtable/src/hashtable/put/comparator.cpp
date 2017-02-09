_CL_VOID
hashtable_put_comparator() {
  ushort inflight_val_size_left = 0;
  bool last_finished = true;

  PutReq show_ahead_fetching_put_req;
  bool is_valid_show_ahead_fetching_put_req = false;

  ulong2 init_data = read_channel_altera(init_hashtable_put_comparator);
  ulong slab_start_addr = init_data.x;
  ulong line_start_addr = init_data.y;
  bool should_forwarding_to_return_put_req = false;
  bool should_forwarding_to_put_offline_handler = false;
  bool should_forwarding_to_put_newline_handler = false;

  while (1) {
    ulong8 line;
    bool read_line;
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
    bool should_write_hashtable_put_slab_return_req_offline = false;
    SlabReturn val_write_hashtable_put_slab_return_req_offline;
    
    if (!is_valid_show_ahead_fetching_put_req) {
      show_ahead_fetching_put_req =
	read_channel_nb_altera(fetching_put_req, &is_valid_show_ahead_fetching_put_req);
    }

    if (is_valid_show_ahead_fetching_put_req) { 
      if (last_finished) {
	line = read_channel_nb_altera(line_fetcher_put_dma_rd_res, &read_line);
	if (read_line) {
	  is_valid_show_ahead_fetching_put_req = false;
	  last_finished = false;
	  PutReq req = show_ahead_fetching_put_req;
	  ulong4 key = req.key;

	  inflight_val_size_left = req.val_size;
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
	  
	  if (!req.del_fin) {
	    bool offline_found = false;
	    uchar offline_found_idx;
	    uchar offline_found_slab_type;
	    ulong offline_found_val_addr;
      
	    // do offline comparasion
#pragma unroll
	    for (int i = 0; i < 10; i ++) {
	      if (line_5B_metadata[i] == 31) { // 5'b11111
		ushort line_hash2 = (((ushort)line_in_uchar[5 * i]) << 2) | (((ushort)line_in_uchar[5 * i + 1]) >> 6);
		if (line_hash2 == req.hash2) {
		  offline_found = true;
		  offline_found_idx = i;
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
	    uchar inline_found_idx_slots[10];
	    uchar inline_found_5B_num_slots[10];
#pragma unroll
	    for (int i = 0; i < 10; i ++) {
	      inline_found_5B_num_slots[i] = 0;
	    }

	    uchar data_to_write[32];
	    // do inline comparasion
	    if (req.key_size <= 12) { // otherwise cannot be inlined, since key_size + val_size <= 15 in inline mode && key_size <= 3 && val_size <= 3	
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
			inline_found_idx_slots[i] = i;
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
		  
			ushort total_length = val_end_idx - val_start_idx + 1 + req.key_size;

			inline_found_5B_num_slots[i] =
			  (total_length <= 10) ? 2 :
			  3;
		  
#pragma unroll
			for (int j = 0; j < 32; j ++) {
			  data_to_write[j] = line_in_uchar[32 + j];
			}
		  
			if (inline_found_5B_num_slots[i] == 2) {
			  if (inline_found_idx_slots[i] == 0) {
			    data_to_write[18] &= 7; // 8'00000111
			    data_to_write[18] &= 248; // 8'b11111000
			    data_to_write[19] &= 63; // 8'b00111111
			  }
			  else if (inline_found_idx_slots[i] == 1) {
			    data_to_write[18] &= 248; // 8'b11111000 
			    data_to_write[19] &= 63; // 8'b00111111
			    data_to_write[19] &= 193; // 8'b11000001
			  }
			  else if (inline_found_idx_slots[i] == 2) {
			    data_to_write[19] &= 193; // 8'b11000001
			    data_to_write[19] &= 255; // 8'11111110
			    data_to_write[20] &= 15; // 8'b00001111
			  }
			  else if (inline_found_idx_slots[i] == 3) {
			    data_to_write[19] &= 255; // 8'11111110
			    data_to_write[20] &= 15; // 8'b00001111
			    data_to_write[20] &= 240; // 8'b11110000
			    data_to_write[21] &= 127; // 8'b01111111
			  }
			  else if (inline_found_idx_slots[i] == 4) {
			    data_to_write[20] &= 240; // 8'b11110000
			    data_to_write[21] &= 127; // 8'b01111111
			    data_to_write[21] &= 131; // 8'b10000011
			  }
			  else if (inline_found_idx_slots[i] == 5) {
			    data_to_write[21] &= 131; // 8'b10000011
			    data_to_write[21] &= 252; // 8'b11111100
			    data_to_write[22] &= 31; // 8'b00011111
			  }
			  else if (inline_found_idx_slots[i] == 6) {
			    data_to_write[21] &= 252; // 8'b11111100
			    data_to_write[22] &= 31; // 8'b00011111
			    data_to_write[22] &= 224; // 8'b11100000
			  }
			  else if (inline_found_idx_slots[i] == 7) {
			    data_to_write[22] &= 224; // 8'b11100000
			    data_to_write[23] &= 7; // 8'b00000111
			  }
			  else if (inline_found_idx_slots[i] == 8) {
			    data_to_write[23] &= 7; // 8'b00000111
			    data_to_write[23] &= 248; // 8'b11111000
			    data_to_write[24] &= 63; // 8'b00111111
			  }
			}
			else if (inline_found_5B_num_slots[i] == 3) {
			  if (inline_found_idx_slots[i] == 0) {
			    data_to_write[18] &= 7; // 8'00000111
			    data_to_write[18] &= 248; // 8'b11111000
			    data_to_write[19] &= 63; // 8'b00111111
			    data_to_write[19] &= 193; // 8'b11000001
			  }
			  else if (inline_found_idx_slots[i] == 1) {
			    data_to_write[18] &= 248; // 8'b11111000
			    data_to_write[19] &= 63; // 8'b00111111
			    data_to_write[19] &= 193; // 8'b11000001
			    data_to_write[19] &= 255; // 8'11111110
			    data_to_write[20] &= 15; // 8'b00001111
			  }
			  else if (inline_found_idx_slots[i] == 2) {
			    data_to_write[19] &= 193; // 8'b11000001
			    data_to_write[19] &= 255; // 8'11111110
			    data_to_write[20] &= 15; // 8'b00001111
			    data_to_write[20] &= 240; // 8'b11110000
			    data_to_write[21] &= 127; // 8'b01111111
			  }
			  else if (inline_found_idx_slots[i] == 3) {
			    data_to_write[19] &= 255; // 8'11111110
			    data_to_write[20] &= 15; // 8'b00001111
			    data_to_write[20] &= 240; // 8'b11110000
			    data_to_write[21] &= 127; // 8'b01111111
			    data_to_write[21] &= 131; // 8'b10000011
			  }
			  else if (inline_found_idx_slots[i] == 4) {
			    data_to_write[20] &= 240; // 8'b11110000
			    data_to_write[21] &= 127; // 8'b01111111
			    data_to_write[21] &= 131; // 8'b10000011
			    data_to_write[21] &= 252; // 8'b11111100
			    data_to_write[22] &= 31; // 8'b00011111
			  }
			  else if (inline_found_idx_slots[i] == 5) {
			    data_to_write[21] &= 131; // 8'b10000011
			    data_to_write[21] &= 252; // 8'b11111100
			    data_to_write[22] &= 31; // 8'b00011111
			    data_to_write[22] &= 224; // 8'b11100000
			  }
			  else if (inline_found_idx_slots[i] == 6) {
			    data_to_write[21] &= 252; // 8'b11111100
			    data_to_write[22] &= 31; // 8'b00011111
			    data_to_write[22] &= 224; // 8'b11100000
			    data_to_write[23] &= 7; // 8'b00000111
			  }
			  else if (inline_found_idx_slots[i] == 7) {
			    data_to_write[22] &= 224; // 8'b11100000
			    data_to_write[23] &= 7; // 8'b00000111
			    data_to_write[23] &= 248; // 8'b11111000
			    data_to_write[24] &= 63; // 8'b00111111
			  }
			}		  
		      }		
		    }
		  }
		}
	      }
	    }

	    bool inline_found = false;
#pragma unroll
	    for (int i = 0; i < 10; i ++) {
	      if (inline_found_slots[i]) {
		inline_found = true;
	      }
	    }
	    
	    if (inline_found) {
	      // have already processed
	    }
	    else if (!inline_found && offline_found) {
	      // delete offline metadata
#pragma unroll
	      for (int i = 0; i < 32; i ++) {
		data_to_write[i] = line_in_uchar[32 + i];
	      }
	
	      if (offline_found_idx == 0) {
		data_to_write[18] &= 7; // 8'00000111
	      }
	      else if (offline_found_idx == 1) {
		data_to_write[18] &= 248; // 8'b11111000
		data_to_write[19] &= 63; // 8'b00111111
	      }
	      else if (offline_found_idx == 2) {
		data_to_write[19] &= 193; // 8'b11000001
	      }
	      else if (offline_found_idx == 3) {
		data_to_write[19] &= 255; // 8'11111110
		data_to_write[20] &= 15; // 8'b00001111
	      }
	      else if (offline_found_idx == 4) {
		data_to_write[20] &= 240; // 8'b11110000
		data_to_write[21] &= 127; // 8'b01111111
	      }
	      else if (offline_found_idx == 5) {
		data_to_write[21] &= 131; // 8'b10000011
	      }
	      else if (offline_found_idx == 6) {
		data_to_write[21] &= 252; // 8'b11111100
		data_to_write[22] &= 31; // 8'b00011111
	      }
	      else if (offline_found_idx == 7) {
		data_to_write[22] &= 224; // 8'b11100000
	      }
	      else if (offline_found_idx == 8) {
		data_to_write[23] &= 7; // 8'b00000111
	      }
	      else if (offline_found_idx == 9) {
		data_to_write[23] &= 248; // 8'b11111000
		data_to_write[24] &= 63; // 8'b00111111
	      }		
	      // return the slab back
	      should_write_hashtable_put_slab_return_req_offline = true;
	      val_write_hashtable_put_slab_return_req_offline.slab_size = 1 << (offline_found_slab_type + 5);
	      val_write_hashtable_put_slab_return_req_offline.slab_addr = offline_found_val_addr + slab_start_addr;
	    }

	    if (inline_found || offline_found) {
	      DMA_WriteReq_Compressed wr_req_compressed;
#pragma unroll
	      for (int i = 0; i < 32; i ++) {
		line_in_uchar[32 + i] = data_to_write[i];
	      }
	      req.del_fin = true;
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
		  (((ulong)data_to_write[0 + (i << 3)]) << 56) |
		  (((ulong)data_to_write[1 + (i << 3)]) << 48) |
		  (((ulong)data_to_write[2 + (i << 3)]) << 40) |
		  (((ulong)data_to_write[3 + (i << 3)]) << 32) |
		  (((ulong)data_to_write[4 + (i << 3)]) << 24) |
		  (((ulong)data_to_write[5 + (i << 3)]) << 16) |
		  (((ulong)data_to_write[6 + (i << 3)]) << 8) |
		  (((ulong)data_to_write[7 + (i << 3)]) << 0);
	      }
	      wr_req_compressed.data.x = tmp[0];
	      wr_req_compressed.data.y = tmp[1];
	      wr_req_compressed.data.z = tmp[2];
	      wr_req_compressed.data.w = tmp[3];
		
	      val_write_put_inline_dma_wr_req_double.x = wr_req_compressed;
	      val_write_put_inline_dma_wr_req_double.valid2 = false;
	    }
	    
	  }// end: if (!del_fin)
	  
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
      }
      else {
	// forwarding show_ahead_fetching_put_req
	is_valid_show_ahead_fetching_put_req = false;
	assert(
	       should_forwarding_to_return_put_req ||
	       should_forwarding_to_put_offline_handler ||
	       should_forwarding_to_put_newline_handler
	       );
	if (should_forwarding_to_return_put_req) {
	  should_write_return_put_req = true;
	  val_write_return_put_req = show_ahead_fetching_put_req;
	}
	else if (should_forwarding_to_put_offline_handler) {
	  should_write_put_offline_handler = true;
	  val_write_put_offline_handler.req = show_ahead_fetching_put_req;
	}
	else if (should_forwarding_to_put_newline_handler) {
	  should_write_put_newline_handler = true;
	  val_write_put_newline_handler.req = show_ahead_fetching_put_req;
	}
      }

      if (!is_valid_show_ahead_fetching_put_req) {
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

      if (should_write_hashtable_put_slab_return_req_offline) {
	bool dummy;
	dummy = write_channel_nb_altera(hashtable_put_slab_return_req_offline, val_write_hashtable_put_slab_return_req_offline);
	assert(dummy);
      }
      
    }
  }
}
