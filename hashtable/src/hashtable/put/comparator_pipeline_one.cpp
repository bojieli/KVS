_CL_VOID
hashtable_put_comparator_pipeline_one() {
  ushort inflight_val_size_left = 0;
  bool last_finished = true;

  PutReq show_ahead_fetching_put_req;
  bool is_valid_show_ahead_fetching_put_req = false;

  ulong2 init_data = read_channel_altera(init_hashtable_put_comparator_pipeline_two);
  ulong slab_start_addr = init_data.x;
  ulong line_start_addr = init_data.y;

  while (1) {
    ulong8 line;
    bool read_line;
    DMA_WriteReq_Compressed_Double val_write_put_inline_dma_wr_req_double;
    PutReq val_write_return_put_req;
    bool should_write_hashtable_put_slab_return_req_offline = false;
    SlabReturn val_hashtable_put_slab_return_req_offline;

    bool should_write_put_comparator_interm_info = false;
    PutComparatorIntermInfo val_put_comparator_interm_info;
    val_put_comparator_interm_info.line_updated = false;
    
    PutReq req;
    uchar line_in_uchar[64];
    
    if (!is_valid_show_ahead_fetching_put_req) {
      show_ahead_fetching_put_req =
	read_channel_nb_altera(fetching_put_req, &is_valid_show_ahead_fetching_put_req);
    }

    if (is_valid_show_ahead_fetching_put_req) {
      req = show_ahead_fetching_put_req;
      if (last_finished) {
	line = read_channel_nb_altera(line_fetcher_put_dma_rd_res, &read_line);
	if (read_line) {
	  is_valid_show_ahead_fetching_put_req = false;
	  last_finished = false;
	  ulong4 key = req.key;

	  inflight_val_size_left = req.val_size;
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
	      val_hashtable_put_slab_return_req_offline.slab_size = 1 << (offline_found_slab_type + 5);
	      val_hashtable_put_slab_return_req_offline.slab_addr = offline_found_val_addr + slab_start_addr;
	    }

	    if (inline_found || offline_found) {
	      DMA_WriteReq_Compressed wr_req_compressed;
#pragma unroll
	      for (int i = 0; i < 32; i ++) {
		line_in_uchar[32 + i] = data_to_write[i];
	      }
	      req.del_fin = true;
	      val_put_comparator_interm_info.line_updated = true;
	    }	    
	  }// end: if (!del_fin)
	  should_write_put_comparator_interm_info = true;
	  val_put_comparator_interm_info.req = req;
#pragma unroll
	  for (int i = 0; i < 64; i ++) {
	    val_put_comparator_interm_info.line[i] = line_in_uchar[i];
	  }
	}
      }
      else {
	is_valid_show_ahead_fetching_put_req = false;
	should_write_put_comparator_interm_info = true;
	val_put_comparator_interm_info.req = req;
      }

      if (!is_valid_show_ahead_fetching_put_req) {
	if (inflight_val_size_left > 32) {
	  inflight_val_size_left -= 32;
	}
	else {
	  inflight_val_size_left = 0;
	  last_finished = true;
	}
      }

      if (should_write_hashtable_put_slab_return_req_offline) {
	bool dummy;
	dummy = write_channel_nb_altera(hashtable_put_slab_return_req_offline, val_hashtable_put_slab_return_req_offline);
	assert(dummy);
      }

      if (should_write_put_comparator_interm_info) {
	bool dummy;
	dummy = write_channel_nb_altera(put_comparator_interm_info, val_put_comparator_interm_info);
	assert(dummy);
      }
      
    }
  }
}
