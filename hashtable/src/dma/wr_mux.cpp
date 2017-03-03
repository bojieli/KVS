_CL_VOID
dma_wr_mux() {
  bool flag = 0;
  ushort inflight_wr_req_left_size = 0;
  uchar inflight_wr_req_id = 0;
  
  while (1) {
    DMA_WriteReq wr_req;
    DMA_WriteReq_Compressed wr_req_compressed;
    bool read_wr_req = false;

    if (!read_wr_req && (inflight_wr_req_id == 1 || inflight_wr_req_id == 0)) {
      wr_req_compressed = read_channel_nb_altera(slab_dma_wr_req, &read_wr_req);
      if (read_wr_req) {
	inflight_wr_req_id = 1;
      }
    }
    
    if (!read_wr_req && (inflight_wr_req_id == 2 || inflight_wr_req_id == 0)) {
      wr_req = read_channel_nb_altera(redirect_pcie_wr_req, &read_wr_req);
      if (read_wr_req) {
	inflight_wr_req_id = 2;
      }
    }

    if (!read_wr_req && (inflight_wr_req_id == 3 || inflight_wr_req_id == 0)) {
      wr_req.raw = read_channel_nb_altera(pcie_wr_req, &read_wr_req);
      if (read_wr_req) {
	inflight_wr_req_id = 3;
      }
    }
        
    if (read_wr_req) {
      
      if (!inflight_wr_req_left_size) {
	inflight_wr_req_left_size = wr_req.req.size;
      }
      
      if (flag) {
	bool dummy = write_channel_nb_altera(dma0_wr_req, wr_req.raw);
	assert(dummy);
      }
      else {
	bool dummy = write_channel_nb_altera(dma1_wr_req, wr_req.raw);
	assert(dummy);
      }
      
      if (inflight_wr_req_left_size > 32) {
	inflight_wr_req_left_size -= 32;
      }
      else {
	inflight_wr_req_left_size = 0;
	inflight_wr_req_id = 0;
	flag ^= 1;
      }
    }
  }
}
