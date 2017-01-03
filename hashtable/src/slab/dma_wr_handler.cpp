_CL_VOID
slab_dma_wr_handler() {
  uchar inflight_wr_req_id;
  ushort inflight_wr_req_size_left = 0;

  bool is_valid_slab_bin_dma_wr_req[SLAB_BIN_COUNT] = {0, 0, 0, 0, 0};
  ulong8 show_ahead_slab_bin_dma_wr_req[SLAB_BIN_COUNT];
  
  while (1) {
#pragma unroll
    for (int i = 0; i < SLAB_BIN_COUNT; i ++) {
      if (!is_valid_slab_bin_dma_wr_req[i]) {
	show_ahead_slab_bin_dma_wr_req[i] = read_channel_nb_altera(slab_bin_dma_wr_req[i], &is_valid_slab_bin_dma_wr_req[i]);
      }
    }
    
    if (!inflight_wr_req_size_left) {
      if (is_valid_slab_bin_dma_wr_req[0]) {
	DMA_WriteReq tmp;
	tmp.raw = show_ahead_slab_bin_dma_wr_req[0];
	inflight_wr_req_size_left = tmp.req.size;
	assert(inflight_wr_req_size_left > 0);
	inflight_wr_req_id = 0;
      }      
      else if (is_valid_slab_bin_dma_wr_req[1]) {
	DMA_WriteReq tmp;
	tmp.raw = show_ahead_slab_bin_dma_wr_req[1];
	inflight_wr_req_size_left = tmp.req.size;
	assert(inflight_wr_req_size_left > 0);	
	inflight_wr_req_id = 1;
      }
      else if (is_valid_slab_bin_dma_wr_req[2]) {
	DMA_WriteReq tmp;
	tmp.raw = show_ahead_slab_bin_dma_wr_req[2];
	inflight_wr_req_size_left = tmp.req.size;
	assert(inflight_wr_req_size_left > 0);
	inflight_wr_req_id = 2;
      }
      else if (is_valid_slab_bin_dma_wr_req[3]) {
	DMA_WriteReq tmp;
	tmp.raw = show_ahead_slab_bin_dma_wr_req[3];
	inflight_wr_req_size_left = tmp.req.size;
	assert(inflight_wr_req_size_left > 0);
	inflight_wr_req_id = 3;
      }
      else if (is_valid_slab_bin_dma_wr_req[4]) {
	DMA_WriteReq tmp;
	tmp.raw = show_ahead_slab_bin_dma_wr_req[4];
	inflight_wr_req_size_left = tmp.req.size;
	assert(inflight_wr_req_size_left > 0);
	inflight_wr_req_id = 4;
      }      
    }

    bool should_write_slab_dma_wr_req = false;
    ulong8 val_write_slab_dma_wr_req;
    
    if (inflight_wr_req_size_left) {
      assert(
	     inflight_wr_req_id == 0 ||
	     inflight_wr_req_id == 1 ||
	     inflight_wr_req_id == 2 ||
	     inflight_wr_req_id == 3 ||
	     inflight_wr_req_id == 4
	     );
      if (inflight_wr_req_id == 0) {
	if (is_valid_slab_bin_dma_wr_req[0]) {
	  is_valid_slab_bin_dma_wr_req[0] = false;
	  should_write_slab_dma_wr_req = true;
	  val_write_slab_dma_wr_req = show_ahead_slab_bin_dma_wr_req[0];
	}
      }
      else if (inflight_wr_req_id == 1) {
	if (is_valid_slab_bin_dma_wr_req[1]) {
	  is_valid_slab_bin_dma_wr_req[1] = false;
	  should_write_slab_dma_wr_req = true;
	  val_write_slab_dma_wr_req = show_ahead_slab_bin_dma_wr_req[1];
	}	
      }
      else if (inflight_wr_req_id == 2) {
	if (is_valid_slab_bin_dma_wr_req[2]) {
	  is_valid_slab_bin_dma_wr_req[2] = false;
	  should_write_slab_dma_wr_req = true;
	  val_write_slab_dma_wr_req = show_ahead_slab_bin_dma_wr_req[2];
	}	
      }
      else if (inflight_wr_req_id == 3) {
	if (is_valid_slab_bin_dma_wr_req[3]) {
	  is_valid_slab_bin_dma_wr_req[3] = false;
	  should_write_slab_dma_wr_req = true;
	  val_write_slab_dma_wr_req = show_ahead_slab_bin_dma_wr_req[3];
	}	
      }
      else if (inflight_wr_req_id == 4) {
	if (is_valid_slab_bin_dma_wr_req[4]) {
	  is_valid_slab_bin_dma_wr_req[4] = false;
	  should_write_slab_dma_wr_req = true;
	  val_write_slab_dma_wr_req = show_ahead_slab_bin_dma_wr_req[4];
	}	
      }

      assert(should_write_slab_dma_wr_req);
      if (should_write_slab_dma_wr_req) {
	if (inflight_wr_req_size_left > 32) {
	  inflight_wr_req_size_left -= 32;
	}
	else {
	  inflight_wr_req_size_left = 0;
	  inflight_wr_req_id = 5; // give an invalid number
	}
	bool dummy = write_channel_nb_altera(slab_dma_wr_req, val_write_slab_dma_wr_req);
      }
      
    }
    
  }
}
