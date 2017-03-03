_CL_VOID
cache_dram_wr_req_splitter() {
  bool state = false;
  Mem_WriteReq_Tmp mem_wrreq;

  ushort inflight_wr_req_size = 0;
  ulong slab_start_addr = read_channel_altera(init_dram_wr_req_splitter);

  ushort inflight_addr_offset = 0;
  
  while (1) {
    bool read_cache_wr_req;
    DMA_WriteReq cache_wrreq;
    cache_wrreq = read_channel_nb_altera(cache_wr_req_raw, &read_cache_wr_req);
    
    if (read_cache_wr_req) {
      bool should_write_cache_wr_req = false;
      
      if (!inflight_wr_req_size) {
	inflight_wr_req_size = cache_wrreq.req.size;
	inflight_addr_offset = 0;
	state = false;
      }
      
      if (!state) {
	mem_wrreq.address = get_cache_address(cache_wrreq.req.address, slab_start_addr) + inflight_addr_offset;
	mem_wrreq.flag = get_cache_flag(cache_wrreq.req.address, slab_start_addr);
	mem_wrreq.flag <<= 3;
	if (cache_wrreq.req.size == 32) {
	  if ((cache_wrreq.req.address - slab_start_addr) & 0x3F) {
	    mem_wrreq.flag |= 2;
	  }
	  else {
	    mem_wrreq.flag |= 4;
	  }
	}
	else {
	  mem_wrreq.flag |= 6;
	}
	if (cache_wrreq.req.size == 32) {
	  mem_wrreq.is_32B = true;
	  state = false;
	  if ((cache_wrreq.req.address - slab_start_addr) & 0x3F) {
	    mem_wrreq.data.hi = cache_wrreq.req.data;
	    mem_wrreq.is_first_half = false;
	  }
	  else {
	    mem_wrreq.data.lo = cache_wrreq.req.data;
	    mem_wrreq.is_first_half = true;
	  }
	  should_write_cache_wr_req = true;
	}
	else {
	  mem_wrreq.is_32B = false;
	  mem_wrreq.data.lo = cache_wrreq.req.data;
	  state = true;
	}
      }
      else {
	mem_wrreq.data.hi = cache_wrreq.req.data;
	state = false;
	should_write_cache_wr_req = true;
      }

      inflight_wr_req_size -= 32;

      if (should_write_cache_wr_req) {      	
	bool dummy = write_channel_nb_altera(cache_wr_req, mem_wrreq);
	assert(dummy);
	inflight_addr_offset ++;
      }
      
    }
  }
}
