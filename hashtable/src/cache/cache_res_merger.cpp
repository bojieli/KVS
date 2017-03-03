_CL_VOID
cache_cache_res_merger() {
  bool is_valid_hit_res = false;
  CacheReadHitRes show_ahead_hit_res;
  bool is_valid_miss_res = false;
  CacheReadMissRes show_ahead_miss_res;
  bool is_valid_swapin_cache_line_addr = false;
  ulong show_ahead_cache_line_addr;

  ulong slab_start_addr = read_channel_altera(init_cache_res_merger);
  
  while (1) {
    if (!is_valid_hit_res) {
      show_ahead_hit_res = read_channel_nb_altera(cache_rd_hit_res, &is_valid_hit_res);
    }
    if (is_valid_hit_res) {
      CacheReadHitRes hit_res = show_ahead_hit_res;
      bool should_write_cache_rd_res = false;
      CacheReadRes cache_rdres;
      if (!hit_res.is_miss) {
	cache_rdres.data = hit_res.data;
	cache_rdres.is_32B = hit_res.is_32B;
	cache_rdres.is_first_half = hit_res.is_first_half;
	should_write_cache_rd_res = true;
	is_valid_hit_res = false;
      }
      else {
	if (!is_valid_miss_res) {
	  show_ahead_miss_res = read_channel_nb_altera(cache_rd_miss_res, &is_valid_miss_res);
	}
	if (!is_valid_swapin_cache_line_addr) {
	  show_ahead_cache_line_addr = read_channel_nb_altera(swapin_cache_line_addr, &is_valid_swapin_cache_line_addr);
	}
	
	if (is_valid_miss_res && is_valid_swapin_cache_line_addr) {	  
	  is_valid_miss_res = false;
	  is_valid_swapin_cache_line_addr = false;
	  is_valid_hit_res = false;
	  
	  CacheReadMissRes miss_res = show_ahead_miss_res;
	  ulong address = show_ahead_cache_line_addr;

	  cache_rdres.data = miss_res.data;
	  cache_rdres.is_32B = miss_res.is_32B;
	  if (miss_res.is_32B) {
	    // in dma/rd_demux.cpp, always put 32B data into
	    // the first half of ulong8
	    cache_rdres.is_first_half = true;
	  }
	  else {
	    cache_rdres.is_first_half = false;
	  }
	  should_write_cache_rd_res = true;

	  if (address != 0) {
	    // should swap in
	    Mem_WriteReq_Tmp mem_wrreq;
	    mem_wrreq.address = get_cache_address(address, slab_start_addr);
	    mem_wrreq.data = miss_res.data;	    
	    mem_wrreq.flag = get_cache_flag(address, slab_start_addr);
	    mem_wrreq.flag <<= 3;
	    if (miss_res.is_32B) {
	      if ((address - slab_start_addr) & 0x3F) {
		mem_wrreq.flag |= 2;
	      }
	      else {
		mem_wrreq.flag |= 4;
	      }
	    }
	    else {
	      mem_wrreq.flag |= 6;
	    }
	    mem_wrreq.is_32B = miss_res.is_32B;
	    if (miss_res.is_32B) {
	      if ((mem_wrreq.address - slab_start_addr) & 0x3F) {
		mem_wrreq.is_first_half = false;
		mem_wrreq.data.hi = mem_wrreq.data.lo;
		mem_wrreq.data.lo.x =
		  mem_wrreq.data.lo.y =
		  mem_wrreq.data.lo.z =
		  mem_wrreq.data.lo.w = 0;
	      }
	      else {
		mem_wrreq.is_first_half = true;
	      }
	    }
	    bool dummy = write_channel_nb_altera(swapin_cache_wr_req, mem_wrreq);
	    assert(dummy);
	  }
	}
      }

      if (should_write_cache_rd_res) {
	bool dummy = write_channel_nb_altera(cache_rd_res, cache_rdres);
	assert(dummy);
      }
      
    }
  }
}

