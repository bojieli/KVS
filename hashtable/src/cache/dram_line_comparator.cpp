_CL_VOID
cache_dram_line_comparator() {
  Mem_ReadRes show_ahead_mem_rd_res;
  bool is_valid_mem_rd_res = false;
  DramReadContext show_ahead_dram_rd_req_context;
  bool is_valid_dram_rd_req_context = false;

  ulong slab_start_addr = read_channel_altera(init_dram_line_comparator);
  
  while (1) {
    if (!is_valid_mem_rd_res) {
      show_ahead_mem_rd_res = read_channel_nb_altera(read_mem_rd_res, &is_valid_mem_rd_res);
    }
    if (!is_valid_dram_rd_req_context) {
      show_ahead_dram_rd_req_context = read_channel_nb_altera(dram_rd_req_context, &is_valid_dram_rd_req_context);
    }

    if (is_valid_mem_rd_res && is_valid_dram_rd_req_context) {
      is_valid_mem_rd_res = false;
      is_valid_dram_rd_req_context = false;
      
      CacheReadHitRes cacheReadRes;
      Mem_ReadRes mem_rdres = show_ahead_mem_rd_res;
      DramReadContext context = show_ahead_dram_rd_req_context;

      bool hit = (mem_rdres.flag >> 3) == get_cache_flag(context.address, slab_start_addr);
      uchar valid = ((mem_rdres.flag >> 1) & 3);
      
      if (context.is_32B) {
	ulong4 data;
	if ((context.address - slab_start_addr) & 0x3F) {
	  if (valid == 2 || valid == 0) {
	    hit = false;
	  }
	}
	else {
	  if (valid == 1 || valid == 0) {
	    hit = false;
	  }
	}
      }
      else {
	ulong4 data = mem_rdres.data.lo;
	data = mem_rdres.data.hi;
	if (valid != 3) {
	  hit = false;
	}
      }
      
      // lowerst bit in mem_rdres,flag is dirty bit
      if (hit) {
	cacheReadRes.data = mem_rdres.data;
	if (context.is_32B) {
	  cacheReadRes.is_32B = true;
	  cacheReadRes.is_first_half = !((context.address - slab_start_addr) & 0x3F);
	}
	else {
	  cacheReadRes.is_32B = false;
	}
	cacheReadRes.is_miss = false;
      }
      else {
	// cache miss, redirect to pcie
	cacheReadRes.is_miss = true;
	DMA_ReadReq_Compressed dma_rdreq;
	dma_rdreq.size = (context.is_32B) ? 32 : 64;
	dma_rdreq.address = context.address;
	bool dummy = write_channel_nb_altera(redirect_rd_req, dma_rdreq);
	ulong swapin_address = (context.should_swapin) ? context.address : 0;
	dummy = write_channel_nb_altera(swapin_cache_line_addr, swapin_address);
	assert(dummy);
      }
      bool dummy = write_channel_nb_altera(cache_rd_hit_res, cacheReadRes);
      assert(dummy);
    }    
  }
}
