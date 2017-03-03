_CL_VOID
cache_dma_wr_filter() {
  ulong slab_start_addr = read_channel_altera(init_dma_wr_filter);
  
  bool inflight_is_cached;
  ulong inflight_wr_req_addr;
  ushort inflight_wr_req_size = 0;
  ulong inflight_wr_req_addr2;
  ushort inflight_wr_req_size2 = 0;
  bool valid2 = false;

  while (1) {
    bool read_wr_req;
    DMA_WriteReq wr_req;
    wr_req.raw = read_channel_nb_altera(dma_wr_req, &read_wr_req);

    if (read_wr_req) {
      if (!inflight_wr_req_size) {
	inflight_wr_req_size = wr_req.req.size;
	inflight_is_cached = should_cache(wr_req.req.address, slab_start_addr);
	inflight_wr_req_addr = wr_req.req.address;
	ushort size_left = cache_slice_boundry_distance(wr_req.req.address, slab_start_addr);
	if (wr_req.req.size <= size_left) {
	  valid2 = false;
	}
	else {
	  valid2 = true;
	  inflight_wr_req_size = size_left;
	  inflight_wr_req_size2 = wr_req.req.size - size_left;
	  inflight_wr_req_addr2 = wr_req.req.address + size_left;
	}
      }

      wr_req.req.address = inflight_wr_req_addr;
      wr_req.req.size = inflight_wr_req_size;
      
      if (inflight_wr_req_size > 32) {
	inflight_wr_req_size -= 32;
      }
      else {
	inflight_wr_req_size = 0;
	if (valid2) {
	  valid2 = false;
	  inflight_wr_req_size = inflight_wr_req_size2;
	  inflight_wr_req_addr = inflight_wr_req_addr2;
	  inflight_is_cached ^= 1;
	}
      }
      
      if (inflight_is_cached) {
	bool dummy = write_channel_nb_altera(cache_wr_req_raw, wr_req);
	assert(dummy);
      }
      else {
	bool dummy = write_channel_nb_altera(pcie_wr_req, wr_req.raw);
	assert(dummy);
      }
    }
    
  }

}
