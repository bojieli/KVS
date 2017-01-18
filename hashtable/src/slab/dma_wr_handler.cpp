_CL_VOID
slab_dma_wr_handler() {
  uchar inflight_wr_req_id = 0;
  ushort inflight_wr_req_size_left = 0;
  
  while (1) {
    bool should_write_slab_dma_wr_req = false;
    bool read_slab_bin_dma_wr_req = false;
    DMA_WriteReq_Compressed wr_req;

    if (!read_slab_bin_dma_wr_req && (inflight_wr_req_id == 0 || inflight_wr_req_id == 1)) {
      wr_req = read_channel_nb_altera(slab_bin_dma_wr_req[0], &read_slab_bin_dma_wr_req);
      if (read_slab_bin_dma_wr_req) {
	inflight_wr_req_id = 1;
      }
    }

    if (!read_slab_bin_dma_wr_req && (inflight_wr_req_id == 0 || inflight_wr_req_id == 2)) {
      wr_req = read_channel_nb_altera(slab_bin_dma_wr_req[1], &read_slab_bin_dma_wr_req);
      if (read_slab_bin_dma_wr_req) {
	inflight_wr_req_id = 2;
      }
    }
    
    if (!read_slab_bin_dma_wr_req && (inflight_wr_req_id == 0 || inflight_wr_req_id == 3)) {
      wr_req = read_channel_nb_altera(slab_bin_dma_wr_req[2], &read_slab_bin_dma_wr_req);
      if (read_slab_bin_dma_wr_req) {
	inflight_wr_req_id = 3;
      }
    }
    
    if (!read_slab_bin_dma_wr_req && (inflight_wr_req_id == 0 || inflight_wr_req_id == 4)) {
      wr_req = read_channel_nb_altera(slab_bin_dma_wr_req[3], &read_slab_bin_dma_wr_req);
      if (read_slab_bin_dma_wr_req) {
	inflight_wr_req_id = 4;
      }
    }
    
    if (!read_slab_bin_dma_wr_req && (inflight_wr_req_id == 0 || inflight_wr_req_id == 5)) {
      wr_req = read_channel_nb_altera(slab_bin_dma_wr_req[4], &read_slab_bin_dma_wr_req);
      if (read_slab_bin_dma_wr_req) {
	inflight_wr_req_id = 5;
      }
    }

    if (read_slab_bin_dma_wr_req) {
      if (!inflight_wr_req_size_left) {
	inflight_wr_req_size_left = wr_req.size;
      }

      if (inflight_wr_req_size_left > 32) {
	inflight_wr_req_size_left -= 32;
      }
      else {
	inflight_wr_req_size_left = 0;
	inflight_wr_req_id = 0; // give an invalid number
      }

      bool dummy = write_channel_nb_altera(slab_dma_wr_req, wr_req);
      assert(dummy);      
    }
  }
}
