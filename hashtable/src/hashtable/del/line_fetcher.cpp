_CL_VOID
hashtable_del_line_fetcher() {
  // retreive the line corresponds to the next chain
  ulong2 init = read_channel_altera(init_hashtable_del_line_fetcher);
  ulong slab_start_addr = init.x;
  ulong line_start_addr = init.y;
  while (1) {
    bool read_del_req;
    DelReq req;
    bool read_from_input;
    req = read_channel_nb_altera(return_del_req, &read_del_req);
    if (!read_del_req) {
      req = read_channel_nb_altera(input_del_req, &read_del_req);
      read_from_input = true;
    }
    else {
      read_from_input = false;
    }
    if (read_del_req) {
      DMA_ReadReq rd_req;
      if (read_from_input) {
	rd_req.req.address = (req.hash1 << 6) + line_start_addr;
	req.has_last = false;
      }
      else {
	rd_req.req.address = (req.hash1 << 5) + slab_start_addr;
	req.has_last = true;
      }
      rd_req.req.size = 64;
      write_channel_altera(line_fetcher_del_dma_rd_req, rd_req.raw);
      write_channel_altera(fetching_del_req, req);
    }
  }
}
