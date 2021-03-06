enum Table_Head_State {
  PART0 = 0,
  PART1,
  PART2,
  PART3,
  PART4,
  PART5,
  PART6,
  PART7,
  EMPTY
};

_CL_VOID
slab_besides_return() {
#define slab_size(i) (1 << ((i) + SLAB_MIN_SIZE_LOG2))
  uint host_slab_available_table_head_ptr_offset_mask[SLAB_BIN_COUNT];
  uint host_slab_available_table_head_ptr_offset[SLAB_BIN_COUNT];
  ulong host_slab_available_table_base_addr[SLAB_BIN_COUNT];
  
  ushort slab_return_table_size[SLAB_BIN_COUNT];
  uint host_slab_return_table_tail_ptr_offset[SLAB_BIN_COUNT];
  uint host_slab_return_table_tail_ptr_offset_mask[SLAB_BIN_COUNT];
  ulong host_slab_return_table_base_addr[SLAB_BIN_COUNT];

  ushort slab_cache_table_size[SLAB_BIN_COUNT]; 
  ulong4 current_cache_table_head[SLAB_BIN_COUNT];
  enum Table_Head_State current_cache_table_head_state[SLAB_BIN_COUNT];
  uchar current_cache_table_head_num[SLAB_BIN_COUNT];
  ulong current_cache_table_head_addr[SLAB_BIN_COUNT];
  
  ushort auto_return_mode[SLAB_BIN_COUNT];  
  ulong4 current_return_table_head[SLAB_BIN_COUNT];
  enum Table_Head_State current_return_table_head_state[SLAB_BIN_COUNT];
    
  ulong slab_start_addr;
  
  ClSignal init_signal;
  init_signal.raw = read_channel_altera(host_init_slab);
  bool dummy = write_channel_nb_altera(init_slab_dma_rd_handler, init_signal.raw);
  assert(dummy);

#pragma unroll
  for (int i = 0; i < SLAB_BIN_COUNT; i ++) {
    host_slab_available_table_base_addr[i] = init_signal.Sig.LParam[i];
  }
  host_slab_return_table_base_addr[0] = host_slab_available_table_base_addr[4] + (1 << (26 + 2));
  host_slab_return_table_base_addr[1] = (host_slab_return_table_base_addr[0] + (1 << (20 + 2)));
  host_slab_return_table_base_addr[2] = (host_slab_return_table_base_addr[1] + (1 << (20 + 2)));
  host_slab_return_table_base_addr[3] = (host_slab_return_table_base_addr[2] + (1 << (20 + 2)));
  host_slab_return_table_base_addr[4] = (host_slab_return_table_base_addr[3] + (1 << (20 + 2)));
  slab_start_addr = host_slab_return_table_base_addr[4] + (1 << (20 + 2));
  
  ulong slab_end_addr = slab_start_addr + (1ULL << 35);

  dummy = write_channel_nb_altera(init_hashtable_get_comparator, slab_start_addr);
  assert(dummy);
  dummy = write_channel_nb_altera(init_slab_return, slab_start_addr);
  assert(dummy);
  dummy = write_channel_nb_altera(init_cache_res_merger, slab_start_addr);
  assert(dummy);
  dummy = write_channel_nb_altera(init_dram_line_comparator, slab_start_addr);
  assert(dummy);
  dummy = write_channel_nb_altera(init_dram_writer, slab_start_addr);
  assert(dummy);
  dummy = write_channel_nb_altera(init_dma_rd_filter, slab_start_addr);
  assert(dummy);
  dummy = write_channel_nb_altera(init_dram_wr_req_splitter, slab_start_addr);
  assert(dummy);
  dummy = write_channel_nb_altera(init_dma_wr_filter, slab_start_addr);
  assert(dummy);
  dummy = write_channel_nb_altera(init_dram_rd_req_splitter, slab_start_addr);
  assert(dummy);

  ulong2 tmp;  
  tmp.x = slab_start_addr;
  tmp.y = slab_end_addr;
  dummy = write_channel_nb_altera(init_hashtable_del_comparator, tmp);
  assert(dummy);
  dummy = write_channel_nb_altera(init_hashtable_put_comparator_pipeline_one, tmp);
  assert(dummy);
  dummy = write_channel_nb_altera(init_hashtable_put_comparator_pipeline_two, tmp);
  assert(dummy);
  dummy = write_channel_nb_altera(init_hashtable_add_comparator, tmp);
  assert(dummy);
  dummy = write_channel_nb_altera(init_hashtable_put_offline_handler, tmp);
  assert(dummy);
  dummy = write_channel_nb_altera(init_hashtable_del_line_fetcher, tmp);
  assert(dummy);
  dummy = write_channel_nb_altera(init_hashtable_get_line_fetcher, tmp);
  assert(dummy);
  dummy = write_channel_nb_altera(init_hashtable_put_line_fetcher, tmp);
  assert(dummy);
  dummy = write_channel_nb_altera(init_hashtable_add_line_fetcher, tmp);
  assert(dummy);
  dummy = write_channel_nb_altera(init_hashtable_put_newline_handler, tmp);
  assert(dummy);
  
  // array init
#pragma unroll
  for (int i = 0; i < SLAB_BIN_COUNT; i ++) {
    slab_return_table_size[i] = 0;
    auto_return_mode[i] = 0;
    host_slab_available_table_head_ptr_offset[i] = SLAB_CACHE_TABLE_MAX_SIZE;
    host_slab_available_table_head_ptr_offset_mask[i] = (1 << (30 - i)) - 1;
    host_slab_return_table_tail_ptr_offset[i] = 0;
    host_slab_return_table_tail_ptr_offset_mask[i] = (1 << 20) - 1;
    slab_cache_table_size[i] = SLAB_CACHE_TABLE_MAX_SIZE;
    current_cache_table_head_num[i] = 0;
    current_cache_table_head_state[i] = PART7;
    current_return_table_head_state[i] = PART7;
  }

  // wait for init_fin signal from slab dma handler
  // which will preload some slab entries
  dummy = read_channel_altera(slab_init_finish);
  assert(dummy);

  while (1) {
    ushort slab_req_size;
    bool auto_return_mode_first = false;
    ulong grant_addr[SLAB_BIN_COUNT];
    ulong4 val_slab_return_table[SLAB_BIN_COUNT];
    bool auto_fetch_mode[SLAB_BIN_COUNT];
	
    SlabRequest slabRequest;
    ClSignal grant_signal, sync_signal;
    bool have_grant_signal = false, have_sync_signal = false;
    bool read_slab_besides_return_req;
    slabRequest = read_channel_nb_altera(slab_besides_return_req, &read_slab_besides_return_req);
    
    // routine, to see whether need perform fetching or returning
#pragma unroll
    for (int i = 0; i < SLAB_BIN_COUNT; i ++) {
      bool dummy, read_add_8_slab_return_table_size;
      dummy = read_channel_nb_altera(add_8_slab_return_table_size[i], &read_add_8_slab_return_table_size);
      if (read_add_8_slab_return_table_size) {
	slab_return_table_size[i] += 8;
      }
      
      if ((!auto_return_mode[i]) && (slab_return_table_size[i] >= SLAB_RETURN_TO_HOST_THRESHOLD)) {
	auto_return_mode[i] = SLAB_RETURN_TO_HOST_SIZE;
	auto_return_mode_first = true;
      }
      auto_fetch_mode[i] = !((slab_cache_table_size[i] >> 10) & 3);

      if ((auto_return_mode[i]) ||
          (read_slab_besides_return_req && slabRequest.cmd == SIGNAL_REQUEST &&
           (slabRequest.slab_size <= slab_size(i) && (i == 0 || slabRequest.slab_size > slab_size(i - 1))) &&
           (!auto_return_mode[i] && slab_return_table_size[i]) && (current_return_table_head_state[i] == PART7))) {
        bool dummy;
	val_slab_return_table[i] = read_channel_nb_altera(slab_return_table[i], &dummy);
	assert(dummy);
      }

      if (auto_return_mode[i]) {
	slab_return_table_size[i] -= 8;
	DMA_WriteReq dma_wr_req;
	DMA_WriteReq_Compressed dma_wr_req_compressed;
	if (auto_return_mode_first) {
	  ulong base_addr =
	    (i == 0) ? host_slab_return_table_base_addr[0] :
	    (i == 1) ? host_slab_return_table_base_addr[1] :
	    (i == 2) ? host_slab_return_table_base_addr[2] :
	    (i == 3) ? host_slab_return_table_base_addr[3] :
	    host_slab_return_table_base_addr[4];

	  dma_wr_req.req.address = (host_slab_return_table_tail_ptr_offset[i] << 2) + base_addr;
	  dma_wr_req.req.size = SLAB_RETURN_TO_HOST_SIZE << 2;
	}
	dma_wr_req.req.data = val_slab_return_table[i];
	dma_wr_req_compressed.data = dma_wr_req.req.data;
	dma_wr_req_compressed.address = dma_wr_req.req.address;
	dma_wr_req_compressed.size = dma_wr_req.req.size;
	bool dummy = write_channel_nb_altera(slab_bin_dma_wr_req[i], dma_wr_req_compressed);
	assert(dummy);
	auto_return_mode[i] -= 8;
	host_slab_return_table_tail_ptr_offset[i] += 8;
	host_slab_return_table_tail_ptr_offset[i] &= host_slab_return_table_tail_ptr_offset_mask[i];

      }
    }
   
    if (read_slab_besides_return_req) {
      if (slabRequest.cmd == SIGNAL_REQUEST) {	
	// request for a new slab
	slab_req_size = slabRequest.slab_size;
	
#pragma unroll
	for (int i = 0; i < SLAB_BIN_COUNT; i ++) {	  
	  // grant the neareast slab
	  if (slab_req_size <= slab_size(i) && (i == 0 || slab_req_size > slab_size(i - 1))) {
	    if (auto_return_mode[i] || !slab_return_table_size[i]) {
	      if (auto_fetch_mode[i]) {
		ulong base_addr =
		  (i == 0) ? host_slab_available_table_base_addr[0] :
		  (i == 1) ? host_slab_available_table_base_addr[1] :
		  (i == 2) ? host_slab_available_table_base_addr[2] :
		  (i == 3) ? host_slab_available_table_base_addr[3] :
		  host_slab_available_table_base_addr[4];

		DMA_ReadReq dma_rd_req;
		dma_rd_req.req.address = (host_slab_available_table_head_ptr_offset[i] << 2) + base_addr;
		dma_rd_req.req.size = SLAB_FETCH_FROM_HOST_SIZE << 2;
		host_slab_available_table_head_ptr_offset[i] += SLAB_FETCH_FROM_HOST_SIZE;
		host_slab_available_table_head_ptr_offset[i] &= host_slab_available_table_head_ptr_offset_mask[i];
		bool dummy;
		DMA_ReadReq_Compressed dma_rd_req_compressed;
		dma_rd_req_compressed.address = dma_rd_req.req.address;
		dma_rd_req_compressed.size = dma_rd_req.req.size;
		dummy = write_channel_nb_altera(slab_bin_dma_rd_req[i], dma_rd_req_compressed);
		assert(dummy);
		slab_cache_table_size[i] += SLAB_FETCH_FROM_HOST_SIZE;
	      }
	      // read from cache table
	      if (!current_cache_table_head_num[i]) {
		if (current_cache_table_head_state[i] == PART7) {
		  #ifdef _CSIM
		  current_cache_table_head[i] = read_channel_altera(slab_cache_table[i]);
		  #else
		  bool dummy;
		  current_cache_table_head[i] = read_channel_nb_altera(slab_cache_table[i], &dummy);
		  assert(dummy);
		  #endif
		  slab_cache_table_size[i] -= 8;

		  current_cache_table_head_num[i] = 
		    ((current_cache_table_head[i].x & 3) == 0) ? 0 : 
		    ((current_cache_table_head[i].x & 3) == 1) ? 1 : 
		    ((current_cache_table_head[i].x & 3) == 2) ? 3 : 
		    7;
		  current_cache_table_head_addr[i] = (current_cache_table_head[i].x & 0xFFFFFFFF) >> 2;
		  current_cache_table_head_state[i] = PART0;
		}
		else {
		  current_cache_table_head_state[i] = 
		    (current_cache_table_head_state[i] == PART0) ? PART1 : 
		    (current_cache_table_head_state[i] == PART1) ? PART2 : 
		    (current_cache_table_head_state[i] == PART2) ? PART3 : 
		    (current_cache_table_head_state[i] == PART3) ? PART4 : 
		    (current_cache_table_head_state[i] == PART4) ? PART5 : 
		    (current_cache_table_head_state[i] == PART5) ? PART6 : 
		    PART7;
		  uint current_cache_table_head_uint_for_grant_slab = 
		    (current_cache_table_head_state[i] == PART1) ? ((current_cache_table_head[i].x >> 32) & 0xFFFFFFFF) : 
		    (current_cache_table_head_state[i] == PART2) ? (current_cache_table_head[i].y & 0xFFFFFFFF) : 
		    (current_cache_table_head_state[i] == PART3) ? ((current_cache_table_head[i].y >> 32) & 0xFFFFFFFF) : 
		    (current_cache_table_head_state[i] == PART4) ? (current_cache_table_head[i].z & 0xFFFFFFFF) : 
		    (current_cache_table_head_state[i] == PART5) ? ((current_cache_table_head[i].z >> 32) & 0xFFFFFFFF) : 
		    (current_cache_table_head_state[i] == PART6) ? (current_cache_table_head[i].w & 0xFFFFFFFF) : 
		    ((current_cache_table_head[i].w >> 32) & 0xFFFFFFFF);

		  current_cache_table_head_addr[i] = (current_cache_table_head_uint_for_grant_slab) >> 2;
		  current_cache_table_head_num[i] = (1 << (current_cache_table_head_uint_for_grant_slab & 3)) - 1;
		}
	      }
	      else {
		current_cache_table_head_num[i] --;
	      }
	      grant_addr[i] = current_cache_table_head_addr[i] << 5;
	      current_cache_table_head_addr[i] += slab_size(i) >> 5; 
	    }
	    else {
	      // read from return table
	      if (current_return_table_head_state[i] == PART7) {
		current_return_table_head[i] = val_slab_return_table[i];
		slab_return_table_size[i] -= 8;
		current_return_table_head_state[i] = PART0;
		grant_addr[i] = current_return_table_head[i].x & 0xFFFFFFFF;
	      }
	      else {
		current_return_table_head_state[i] = 
		  (current_return_table_head_state[i] == PART0) ? PART1 : 
		  (current_return_table_head_state[i] == PART1) ? PART2 : 
		  (current_return_table_head_state[i] == PART2) ? PART3 : 
		  (current_return_table_head_state[i] == PART3) ? PART4 : 
		  (current_return_table_head_state[i] == PART4) ? PART5 : 
		  (current_return_table_head_state[i] == PART5) ? PART6 :
		  PART7;
		grant_addr[i] = 
		  (current_return_table_head_state[i] == PART1) ? ((current_return_table_head[i].x >> 32) & 0xFFFFFFFF) : 
		  (current_return_table_head_state[i] == PART2) ? (current_return_table_head[i].y & 0xFFFFFFFF) : 
		  (current_return_table_head_state[i] == PART3) ? ((current_return_table_head[i].y >> 32) & 0xFFFFFFFF) : 
		  (current_return_table_head_state[i] == PART4) ? (current_return_table_head[i].z & 0xFFFFFFFF) : 
		  (current_return_table_head_state[i] == PART5) ? ((current_return_table_head[i].z >> 32) & 0xFFFFFFFF) : 
		  (current_return_table_head_state[i] == PART6) ? (current_return_table_head[i].w & 0xFFFFFFFF) : 
		  ((current_return_table_head[i].w >> 32) & 0xFFFFFFFF);
	      }
	      grant_addr[i] >>= 2;
	      grant_addr[i] <<= 5;
	    }
	  }
	}
#pragma unroll
	for (int i = 0; i < SLAB_BIN_COUNT; i ++) {
	  if (slab_req_size <= slab_size(i) && (i == 0 || slab_req_size > slab_size(i - 1))) {
	    grant_signal.Sig.Cmd = SIGNAL_GRANT;
	    grant_signal.Sig.LParam[0] = grant_addr[i] + slab_start_addr;
	    have_grant_signal = true;
	  }
	}
      }
      else if (slabRequest.cmd == SIGNAL_QUERY_AVAILABLE_HEAD_PTR) {
#pragma unroll
	for (int i = 0; i < SLAB_BIN_COUNT; i++) {
	  sync_signal.Sig.Cmd = SIGNAL_RESP_AVAILABLE_HEAD_PTR;
	  sync_signal.Sig.LParam[i] = host_slab_available_table_head_ptr_offset[i];
	  have_sync_signal = true;
	}
      }
      else if (slabRequest.cmd == SIGNAL_QUERY_RETURN_TAIL_PTR) {
#pragma unroll
	for (int i = 0; i < SLAB_BIN_COUNT; i++) {
	  sync_signal.Sig.Cmd = SIGNAL_RESP_RETURN_TAIL_PTR;
	  sync_signal.Sig.LParam[i] = host_slab_return_table_tail_ptr_offset[i];
	  have_sync_signal = true;
	}
      }
      if (have_grant_signal) {
	dummy = write_channel_nb_altera(slab_besides_return_res, grant_signal.Sig.LParam[0]);
	assert(dummy);
      }
      if (have_sync_signal) {
	dummy = write_channel_nb_altera(slab_besides_return_sync, sync_signal.raw);
	assert(dummy);
      }
    }
  }
#undef slab_size
}
