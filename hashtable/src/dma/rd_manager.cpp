// multiplex dma rd channels
_CL_VOID
dma_rd_manager() {
  uchar dma_rd_req_id;
  ushort inflight_rd_res_left_size = 0;
  uchar inflight_rd_res_id;
  ulong8 cache_miss_res_data;
  CacheReadMissRes cache_miss_res;
  bool cache_miss_res_data_state = false;

  while (1) {
    // dma read request
    DMA_ReadReq_Extended dma_rd_req_extended;
    DMA_ReadReq_Compressed dma_rd_req_compressed;
    bool read_dma_rd_req;
    bool should_swapin = false;
    
    dma_rd_req_extended = read_channel_nb_altera(line_fetcher_dma_rd_req, &read_dma_rd_req);
    if (!read_dma_rd_req) {
      dma_rd_req_compressed = read_channel_nb_altera(slab_fetcher_get_offline_dma_rd_req, &read_dma_rd_req);
      if (!read_dma_rd_req) {
	dma_rd_req_compressed = read_channel_nb_altera(slab_fetcher_add_offline_dma_rd_req, &read_dma_rd_req);
	if (!read_dma_rd_req) {
	  // read from other channels
	}
	else {
	  dma_rd_req_id = 3;
	  dma_rd_req_extended.address = dma_rd_req_compressed.address;
	  dma_rd_req_extended.size = dma_rd_req_compressed.size;
	  dma_rd_req_extended.should_swapin = false;
	}
      }
      else {
	dma_rd_req_id = 2;
	dma_rd_req_extended.address = dma_rd_req_compressed.address;
	dma_rd_req_extended.size = dma_rd_req_compressed.size;
	dma_rd_req_extended.should_swapin = false;
      }
    }
    else {
      dma_rd_req_id = 1;
    }

    if (read_dma_rd_req) {
      // save context, used for demuxing when get response
      bool dummy;
      DmaContext context;
      context.size = (ushort)dma_rd_req_extended.size;
      context.id = dma_rd_req_id;
      dummy = write_channel_nb_altera(dma_manager_rd_req_context, context);
      assert(dummy);
      dummy = write_channel_nb_altera(dma_rd_req, dma_rd_req_extended);
      assert(dummy);
    }

    // dma read response
    bool read_dma_rd_res;
    ulong4 dma_read_res = read_channel_nb_altera(dma_rd_res, &read_dma_rd_res);
    
    if (read_dma_rd_res) {
      if (!inflight_rd_res_left_size) {
	bool dummy;
	DmaContext context;
	context = read_channel_nb_altera(dma_manager_rd_req_context, &dummy);
	assert(dummy);
	inflight_rd_res_left_size = context.size;
	inflight_rd_res_id = context.id;
      }
      DMA_ReadRes tmp;
      tmp.raw = dma_read_res;
      if (inflight_rd_res_left_size > 32) {
	inflight_rd_res_left_size -= 32;
      }
      else {
	inflight_rd_res_left_size = 0;
      }
      
      if (inflight_rd_res_id == 1) {
	bool dummy;
	dummy = write_channel_nb_altera(line_fetcher_dma_rd_res, tmp.res.data);
	assert(dummy);
      }
      else if (inflight_rd_res_id == 2) {
	bool dummy;
	dummy = write_channel_nb_altera(slab_fetcher_get_offline_dma_rd_res, tmp.res.data);
	assert(dummy);
      }
      else if (inflight_rd_res_id == 3) {
	bool dummy;
	dummy = write_channel_nb_altera(slab_fetcher_add_offline_dma_rd_res, tmp.res.data);
	assert(dummy);
      }
      else {
	// other channels
      }
    }
  }
}
