_CL_VOID
hashtable_line_fetcher_dma_rd_handler() {
  ushort inflight_rd_res_size_left = 0;
  uchar inflight_rd_res_id;

  ulong8 rd_res_in_ulong8;
  bool state = false;
  
  while (1) {    // issue request
    DMA_ReadReq rd_req;
    bool read_rd_req;
    DmaContext context;
    rd_req.raw = read_channel_nb_altera(line_fetcher_get_dma_rd_req, &read_rd_req);
    if (read_rd_req) {
      context.id = 0;
      context.size = rd_req.req.size;
    }
    else {
      rd_req.raw = read_channel_nb_altera(line_fetcher_del_dma_rd_req, &read_rd_req);
      if (read_rd_req) {
	context.id = 1;
	context.size = rd_req.req.size;
      }
      else {
	rd_req.raw = read_channel_nb_altera(line_fetcher_put_dma_rd_req, &read_rd_req);
	if (read_rd_req) {
	  context.id = 2;
	  context.size = rd_req.req.size;
	}
	else {
	  // read from other channels
	}
      }
    }

    if (read_rd_req) {
      write_channel_altera(line_fetcher_dma_rd_req, rd_req.raw);
      bool dummy = write_channel_nb_altera(line_fetcher_dma_rd_handler_context, context);
    }
    
    // retrieve response
    bool read_rd_res;
    ulong4 rd_res;
    rd_res = read_channel_nb_altera(line_fetcher_dma_rd_res, &read_rd_res);
    if (read_rd_res) {
      if (!inflight_rd_res_size_left) {
	DmaContext context;
	bool dummy;
	context = read_channel_nb_altera(line_fetcher_dma_rd_handler_context, &dummy);
	assert(dummy);
	inflight_rd_res_size_left = context.size;
	inflight_rd_res_id = context.id;
      }
      
      if (inflight_rd_res_size_left > 32) {
	inflight_rd_res_size_left -= 32;
      }
      else {
	inflight_rd_res_size_left = 0;
      }
      
      if (inflight_rd_res_id == 0) {
	if (!state) {
	  rd_res_in_ulong8.lo = rd_res;
	  state = true;
	}
	else {
	  state = false;
	  rd_res_in_ulong8.hi = rd_res;
	  bool dummy = write_channel_nb_altera(line_fetcher_get_dma_rd_res, rd_res_in_ulong8);
	}
      }
      else if (inflight_rd_res_id == 1) {
	if (!state) {
	  rd_res_in_ulong8.lo = rd_res;
	  state = true;
	}
	else {
	  state = false;
	  rd_res_in_ulong8.hi = rd_res;
	  bool dummy = write_channel_nb_altera(line_fetcher_del_dma_rd_res, rd_res_in_ulong8);
	}
      }
      else if (inflight_rd_res_id == 2) {
	if (!state) {
	  rd_res_in_ulong8.lo = rd_res;
	  state = true;
	}
	else {
	  state = false;
	  rd_res_in_ulong8.hi = rd_res;
	  bool dummy = write_channel_nb_altera(line_fetcher_put_dma_rd_res, rd_res_in_ulong8);
	}
      }      
      else {
	// other channels
      }
      
    }
    
  }
}