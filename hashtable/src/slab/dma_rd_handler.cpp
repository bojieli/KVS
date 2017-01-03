_CL_VOID
slab_dma_rd_handler() {
  ushort inflight_rd_res_size_left = 0;
  uchar inflight_rd_res_id;
  uchar req_fin_cnt = 0;

  ulong host_slab_available_table_base_addr[SLAB_BIN_COUNT];
  ClSignal init_signal;
  init_signal.raw = read_channel_altera(init_slab_dma_rd_handler);

#pragma unroll
  for (int i = 0; i < SLAB_BIN_COUNT; i++) {
    host_slab_available_table_base_addr[i] = init_signal.Sig.LParam[i];
  }

  // perform initialization first
  // get some slabs for each bin beforehan
#pragma unroll
  for (int i = 0; i < SLAB_BIN_COUNT; i ++) {
    DMA_ReadReq init_dma_rd_req;
    init_dma_rd_req.req.address = host_slab_available_table_base_addr[i];
    // each slab entry is 4B large(30b for addr, 2b for metadata)
    init_dma_rd_req.req.size = SLAB_CACHE_TABLE_MAX_SIZE << 2;
    bool dummy;
    DmaReadReqWithId dmaReadReqWithId;
    dmaReadReqWithId.req = init_dma_rd_req;
    dmaReadReqWithId.id = (uchar)i;
    dummy = write_channel_nb_altera(slab_init_dma_rd_req_with_id, dmaReadReqWithId);
  }

  while (1) {
    DMA_ReadReq dma_rd_req;
    bool should_write_slab_dma_rd_req = false;
    uchar rd_req_slab_bin_id;
    bool should_read_slab_dma_rd_res = false;
    // already finish initializing, execute core logic
    bool read_slab_bin_dma_rd_req;
    dma_rd_req.raw = read_channel_nb_altera(slab_bin_dma_rd_req[0], &read_slab_bin_dma_rd_req);
    if (read_slab_bin_dma_rd_req) {
      rd_req_slab_bin_id = 0;
      should_write_slab_dma_rd_req = true;
    }
    else {
      dma_rd_req.raw = read_channel_nb_altera(slab_bin_dma_rd_req[1], &read_slab_bin_dma_rd_req);
      if (read_slab_bin_dma_rd_req) {
	rd_req_slab_bin_id = 1;
	should_write_slab_dma_rd_req = true;
      }
      else {
	dma_rd_req.raw = read_channel_nb_altera(slab_bin_dma_rd_req[2], &read_slab_bin_dma_rd_req);
	if (read_slab_bin_dma_rd_req) {
	  rd_req_slab_bin_id = 2;
	  should_write_slab_dma_rd_req = true;
	}
	else {
	  dma_rd_req.raw = read_channel_nb_altera(slab_bin_dma_rd_req[3], &read_slab_bin_dma_rd_req);
	  if (read_slab_bin_dma_rd_req) {
	    rd_req_slab_bin_id = 3;
	    should_write_slab_dma_rd_req = true;
	  }
	  else {
	    dma_rd_req.raw = read_channel_nb_altera(slab_bin_dma_rd_req[4], &read_slab_bin_dma_rd_req);
	    if (read_slab_bin_dma_rd_req) {
	      rd_req_slab_bin_id = 4;
	      should_write_slab_dma_rd_req = true;
	    }
	  }
	}
      }
    }

    if (should_write_slab_dma_rd_req) {
      bool dummy;
      DmaReadReqWithId dmaReadReqWithId;
      dmaReadReqWithId.req = dma_rd_req;
      dmaReadReqWithId.id = rd_req_slab_bin_id;
      dummy = write_channel_nb_altera(slab_non_init_dma_rd_req_with_id, dmaReadReqWithId);
    }

    if (!inflight_rd_res_size_left) {
      if (req_fin_cnt == SLAB_BIN_COUNT) {
	// unblock the slab kernel
	bool dummy;
	dummy = write_channel_nb_altera(slab_init_finish, true);
	req_fin_cnt ++; // make this block be executed only once
      }
      bool read_slab_dma_handler_rd_req_context, dummy;
      DmaContext context = read_channel_nb_altera(slab_dma_handler_rd_req_context, &read_slab_dma_handler_rd_req_context);
      if (read_slab_dma_handler_rd_req_context) {
	should_read_slab_dma_rd_res = true;
	inflight_rd_res_size_left = context.size;
	inflight_rd_res_id = context.id;
	// judge whether initialization is finished
	if (req_fin_cnt < SLAB_BIN_COUNT) {
	  req_fin_cnt ++;
	}
      }
      else {
	inflight_rd_res_size_left = 0;
      }
    }
    else {
      should_read_slab_dma_rd_res = true;
    }

    if (should_read_slab_dma_rd_res) {
      bool read_slab_dma_rd_res;
      ulong4 val_slab_dma_rd_res;
      val_slab_dma_rd_res = read_channel_nb_altera(slab_dma_rd_res, &read_slab_dma_rd_res);
      if (read_slab_dma_rd_res) {
	if (inflight_rd_res_size_left > 32) {
	  inflight_rd_res_size_left -= 32;
	}
	else {
	  inflight_rd_res_size_left = 0;
	}
#pragma unroll
	for (int i = 0; i < SLAB_BIN_COUNT; i++) {
	  if (inflight_rd_res_id == i) {
	    bool dummy;
	    dummy = write_channel_nb_altera(slab_cache_table[i], val_slab_dma_rd_res);
	  }
	}
      }
    }
  }
}

