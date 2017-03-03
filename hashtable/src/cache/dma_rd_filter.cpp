_CL_VOID
cache_dma_rd_filter() {
  ulong slab_start_addr = read_channel_altera(init_dma_rd_filter);

  bool is_valid_rd_req = false;
  DMA_ReadReq_Extended show_ahead_rd_req;
  
  while (1) {
    if (!is_valid_rd_req) {
      show_ahead_rd_req = read_channel_nb_altera(dma_rd_req, &is_valid_rd_req);
    }
    
    if (is_valid_rd_req) {
      DMA_ReadReq_Extended rd_req = show_ahead_rd_req;
      bool is_cached = should_cache(rd_req.address, slab_start_addr);
      ushort size_left = cache_slice_boundry_distance(rd_req.address, slab_start_addr);
      
      if (rd_req.size <= size_left) {
	is_valid_rd_req = false;
      }
      else {
	show_ahead_rd_req.address += size_left;
	show_ahead_rd_req.size -= size_left;	
      }
      
      if (is_cached) {
	bool dummy = write_channel_nb_altera(cache_rd_req, rd_req);
	assert(dummy);
      }
      else {
	DMA_ReadReq dma_rdreq;
	dma_rdreq.req.address = rd_req.address;
	dma_rdreq.req.size = rd_req.size;
	bool dummy = write_channel_nb_altera(pcie_rd_req, dma_rdreq.raw);
	assert(dummy);
      }

      DmaReadFilterContext context;
      context.is_cached = is_cached;
      context.size = rd_req.size;
      bool dummy = write_channel_nb_altera(dma_rd_filter_context, context);
      assert(dummy);
    }
  }
}
