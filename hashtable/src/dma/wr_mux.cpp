_CL_VOID
dma_wr_mux() {
  bool flag = 0;
  ushort inflight_wr_req_left_size = 0;
  while (1) {
    DMA_WriteReq wr_req;
    bool read_wr_req = false;
    wr_req.raw = read_channel_nb_altera(dma_wr_req, &read_wr_req);
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
	flag ^= 1;
      }
    }
  }
}
