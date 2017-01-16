// multiplex dma rd channels
_CL_VOID
dma_rd_manager() {
  uchar dma_rd_req_id;
  ushort inflight_rd_res_left_size = 0;
  uchar inflight_rd_res_id;

  while (1) {
    // dma read request
    ulong8 dma_read_req;
    bool read_dma_rd_req;
    dma_read_req = read_channel_nb_altera(slab_dma_rd_req, &read_dma_rd_req);
    if (!read_dma_rd_req) {
      dma_read_req = read_channel_nb_altera(line_fetcher_dma_rd_req, &read_dma_rd_req);
      if (!read_dma_rd_req) {
	dma_read_req = read_channel_nb_altera(slab_fetcher_get_offline_dma_rd_req, &read_dma_rd_req);
	if (!read_dma_rd_req) {
	  dma_read_req = read_channel_nb_altera(slab_fetcher_add_offline_dma_rd_req, &read_dma_rd_req);
	  if (!read_dma_rd_req) {
	    // other channels
	  }
	  else {
	    dma_rd_req_id = 4;
	  }
	}
	else {
	  dma_rd_req_id = 3;
	}
      }
      else {
	dma_rd_req_id = 2;
      }
    }
    else {
      dma_rd_req_id = 1;
    }

    if (read_dma_rd_req) {
      DMA_ReadReq tmp;
      tmp.raw = dma_read_req;
      // save context, used for demuxing when get response
      bool dummy;
      DmaContext context;
      context.size = (ushort)tmp.req.size;
      context.id = dma_rd_req_id;
      dummy = write_channel_nb_altera(dma_manager_rd_req_context, context);
      assert(dummy);
      dummy = write_channel_nb_altera(dma_rd_req, dma_read_req);
      assert(dummy);
    }

    // dma read response
    bool read_dma_rd_res;
    ulong8 dma_read_res = read_channel_nb_altera(dma_rd_res, &read_dma_rd_res);
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
	dummy = write_channel_nb_altera(slab_dma_rd_res, tmp.res.data);
	assert(dummy);
      }
      else if (inflight_rd_res_id == 2) {
	bool dummy;
	dummy = write_channel_nb_altera(line_fetcher_dma_rd_res, tmp.res.data);
	assert(dummy);
      }
      else if (inflight_rd_res_id == 3) {
	bool dummy;
	dummy = write_channel_nb_altera(slab_fetcher_get_offline_dma_rd_res, tmp.res.data);
	assert(dummy);
      }
      else if (inflight_rd_res_id == 4) {
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
