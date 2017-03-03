_CL_VOID
dma_rd_mux() {
  bool flag = 0;
  
  while (1) {
    DMA_ReadReq rd_req;
    DMA_ReadReq_Compressed rd_req_compressed;
    bool read_rd_req = false;
    bool is_redirect;
    uchar id;

    rd_req_compressed = read_channel_nb_altera(slab_dma_rd_req, &read_rd_req);
    if (!read_rd_req) {
      rd_req_compressed = read_channel_nb_altera(redirect_rd_req, &read_rd_req);    
      if (!read_rd_req) {
	rd_req.raw = read_channel_nb_altera(pcie_rd_req, &read_rd_req);
	if (read_rd_req) {
	  id = 3;
	}
      }
      else {
	rd_req.req.size = rd_req_compressed.size;
	rd_req.req.address = rd_req_compressed.address;;
	id = 2;
      }
    }
    else {
      rd_req.req.size = rd_req_compressed.size;
      rd_req.req.address = rd_req_compressed.address;
      id = 1;
    }
    
    if (read_rd_req) {      
      RdMuxContext context;
      context.id = id;
      context.size = rd_req.req.size;

      bool dummy = write_channel_nb_altera(rd_mux_context, context);
      assert(dummy);
      
      if (flag) {
	bool dummy = write_channel_nb_altera(dma0_rd_req, rd_req.raw);
	assert(dummy);
      }
      else {
	bool dummy = write_channel_nb_altera(dma1_rd_req, rd_req.raw);
	assert(dummy);
      }
      flag ^= 1;
    }
  }
}




