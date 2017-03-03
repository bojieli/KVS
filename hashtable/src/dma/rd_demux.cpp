_CL_VOID
dma_rd_demux() {
  bool flag = 0;
  uchar inflight_id = false;
  ushort inflight_wr_req_left_size = 0;
  CacheReadMissRes cache_miss_res;
  bool cache_miss_res_data_state = false;
  
  while (1) {
    bool read_context;    
    
    if (!inflight_wr_req_left_size) {
      RdMuxContext context = read_channel_nb_altera(rd_mux_context, &read_context);
      if (read_context) {
	inflight_wr_req_left_size = context.size;
	inflight_id = context.id;
	cache_miss_res.is_32B = (context.size == 32);
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

	assert(inflight_id >= 1 && inflight_id <= 3);

	if (inflight_id == 3) {
	  bool dummy = write_channel_nb_altera(pcie_rd_res, data);
	  assert(dummy);
	}
	else if (inflight_id == 2) {
	  bool should_write_cache_rd_miss_res = false;
	  
	  if (cache_miss_res.is_32B) {
	    cache_miss_res_data_state = false;
	    cache_miss_res.data.lo = data;	    
	    cache_miss_res.data.hi.x =
	      cache_miss_res.data.hi.y =
	      cache_miss_res.data.hi.z =
	      cache_miss_res.data.hi.w = 0;
	    should_write_cache_rd_miss_res = true;
	  }
	  else {
	    if (!cache_miss_res_data_state) {
	      cache_miss_res.data.lo = data;
	    }
	    else {
	      cache_miss_res.data.hi = data;
	      should_write_cache_rd_miss_res = true;
	    }
	    cache_miss_res_data_state ^= 1;
	  }
	  if (should_write_cache_rd_miss_res) {
	    bool dummy = write_channel_nb_altera(cache_rd_miss_res, cache_miss_res);
	    assert(dummy);
	  }	  
	}
	else if (inflight_id == 1) {
	  bool dummy = write_channel_nb_altera(slab_dma_rd_res, data);
	  assert(dummy);
	}
	
      }
    }
  }
}
