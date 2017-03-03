_CL_VOID
cache_dma_res_merger() {
  bool is_valid_context = false;
  DmaReadFilterContext show_ahead_context;

  bool is_valid_cache_rd_res = false;
  CacheReadRes show_ahead_cache_rd_res;

  bool state = false;    
  ushort inflight_size = 0;
  
  while (1) {

    if (!is_valid_context) {
      show_ahead_context = read_channel_nb_altera(dma_rd_filter_context, &is_valid_context);      
    }
    
    if (is_valid_context) {
      DmaReadFilterContext context = show_ahead_context;

      ulong4 rd_res;
      if (!inflight_size) {
	inflight_size = context.size;
      }

      bool got_res = false;
      if (context.is_cached) {
	if (!is_valid_cache_rd_res) {
	  show_ahead_cache_rd_res = read_channel_nb_altera(cache_rd_res, &is_valid_cache_rd_res);
	}
	
	if (is_valid_cache_rd_res) {
	  CacheReadRes res = show_ahead_cache_rd_res;
	  got_res = true;

	  if (res.is_32B) {
	    assert(!state);
	    if (res.is_first_half) {
	      rd_res = res.data.lo;
	    }
	    else {
	      rd_res = res.data.hi;
	    }
	    is_valid_cache_rd_res = false;
	  }
	  else {
	    if (!state) {
	      rd_res = res.data.lo;
	    }
	    else {
	      rd_res = res.data.hi;
	      is_valid_cache_rd_res = false;
	    }
	    state ^= 1;
	  }	  
	}
      }
      else {
	bool read_pcie_rd_res;
	ulong4 res;
	res = read_channel_nb_altera(pcie_rd_res, &read_pcie_rd_res);
	if (read_pcie_rd_res) {
	  got_res = true;
	  rd_res = res;
	}
      }

      if (got_res) {
	if (inflight_size > 32) {
	  inflight_size -= 32;
	}
	else {
	  inflight_size = 0;
	  is_valid_context = false;
	}

	bool dummy = write_channel_nb_altera(dma_rd_res, rd_res);
	assert(dummy);
      }
      
    }
  }
}
