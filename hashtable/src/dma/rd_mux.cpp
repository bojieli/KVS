_CL_VOID
dma_rd_mux() {
  bool flag = 0;
  while (1) {
    DMA_ReadReq rd_req;
    bool read_rd_req = false;
    rd_req.raw = read_channel_nb_altera(dma_rd_req, &read_rd_req);
    if (read_rd_req) {
      if (flag) {
	bool dummy = write_channel_nb_altera(dma0_rd_req, rd_req.raw);
	assert(dummy);
      }
      else {
	bool dummy = write_channel_nb_altera(dma1_rd_req, rd_req.raw);
	assert(dummy);
      }
      bool dummy = write_channel_nb_altera(dma_rd_req_size, (ushort)rd_req.req.size);
      assert(dummy);
      flag ^= 1;
    }
  }
}




