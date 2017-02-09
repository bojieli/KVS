_CL_VOID
dma_rd_demux() {
  bool flag = 0;
  ushort inflight_wr_req_left_size = 0;
  while (1) {
    bool read_dma_rd_req_size = false;
    if (!inflight_wr_req_left_size) {
      inflight_wr_req_left_size = read_channel_nb_altera(dma_rd_req_size, &read_dma_rd_req_size);
      if (!read_dma_rd_req_size) {
	inflight_wr_req_left_size = 0;
      }
    }
    if (inflight_wr_req_left_size) {
      bool read_dma_rd_res = false;
      ulong4 data;
      if (flag) {
	data = read_channel_nb_altera(dma0_rd_res, &read_dma_rd_res);
      }
      else {
	data = read_channel_nb_altera(dma1_rd_res, &read_dma_rd_res);
      }
      if (read_dma_rd_res) {
	if (inflight_wr_req_left_size > 32)  {
	  inflight_wr_req_left_size -= 32;
	}
	else {
	  inflight_wr_req_left_size = 0;
	  flag ^= 1;
	}
	bool dummy;
	dummy = write_channel_nb_altera(dma_rd_res, data);
	assert(dummy);
      }
    }
  }

}
