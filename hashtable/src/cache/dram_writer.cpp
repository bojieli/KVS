_CL_VOID
cache_dram_writer() {
  bool is_valid_dram_writer_context = false;
  DramWriterContext show_ahead_dram_writer_context;
  bool is_valid_write_mem_rd_res = false;
  Mem_ReadRes show_ahead_write_mem_rd_res;

  ulong slab_start_addr = read_channel_altera(init_dram_writer);

  while (1) {
    // phase 1: select a write req
    bool read_wr_req;
    Mem_WriteReq_Tmp mem_wrreq;
    bool is_swapin;
    mem_wrreq = read_channel_nb_altera(swapin_cache_wr_req, &read_wr_req);
    if (!read_wr_req) {
      mem_wrreq = read_channel_nb_altera(cache_wr_req, &read_wr_req);
      if (read_wr_req) {
	is_swapin = false;
      }
    }
    else {
      is_swapin = true;
    }
    
    if (read_wr_req) {
      // issue read req first to check flag field of corresponding line in dram
      Mem_ReadReq mem_rdreq;
      mem_rdreq.address = mem_wrreq.address;
      bool dummy = write_channel_nb_altera(write_mem_rd_req, mem_rdreq);
      assert(dummy);
      DramWriterContext context;
      context.wr_req = mem_wrreq;
      context.is_swapin = is_swapin;      
      dummy = write_channel_nb_altera(dram_writer_context, context);
      assert(dummy);
    }

    // phase 2: judge flag field and line updating
    if (!is_valid_dram_writer_context) {
      show_ahead_dram_writer_context = read_channel_nb_altera(dram_writer_context, &is_valid_dram_writer_context);
    }

    if (!is_valid_write_mem_rd_res) {
      show_ahead_write_mem_rd_res = read_channel_nb_altera(write_mem_rd_res, &is_valid_write_mem_rd_res);
    }

    if (is_valid_dram_writer_context && is_valid_write_mem_rd_res) {      
      is_valid_dram_writer_context = false;
      is_valid_write_mem_rd_res = false;
      DramWriterContext context = show_ahead_dram_writer_context;      
      Mem_ReadRes rd_res = show_ahead_write_mem_rd_res;
      Mem_WriteReq wr_req;
      bool should_write_mem_wr_req = false;
      
      RedirectPcieWrReqRaw redirectPcieWrReq;
      bool should_write_redirect_pcie_wr_req = false;

      wr_req.address = context.wr_req.address;
      wr_req.flag = context.wr_req.flag;
      
      if (context.is_swapin) {	
	wr_req.data = context.wr_req.data;
	if (rd_res.flag & 1) {
	  // dirty, then write back to host mem
	  redirectPcieWrReq.address = get_pcie_address((uchar)(rd_res.flag >> 3), context.wr_req.address, slab_start_addr);
	  redirectPcieWrReq.data = rd_res.data;
	  uchar valid = ((rd_res.flag >> 1) & 3);
	  if (valid == 3) {
	    redirectPcieWrReq.is_32B = false;
	  }
	  else {
	    redirectPcieWrReq.is_32B = true;
	    assert(valid);
	    if (valid == 1) {
	      redirectPcieWrReq.is_first_half = false;
	    }
	    else if (valid == 2) {
	      redirectPcieWrReq.is_first_half = true;
	    }
	  }
	  should_write_redirect_pcie_wr_req = true;
	}
	should_write_mem_wr_req = true;
      }
      else {
	bool hit = (rd_res.flag >> 3) == (context.wr_req.flag >> 3);
	if (context.wr_req.is_32B) {
	  if (context.wr_req.is_first_half) {
	    if (!((rd_res.flag >> 2) & 1)) {
	      hit = false;
	    }
	  }
	  else {
	    if (!((rd_res.flag >> 1) & 1)) {
	      hit = false;
	    }
	  }
	}
	else {
	  if (((rd_res.flag >> 1) & 3) != 3) {
	    hit = false;
	  }
	}

	if (hit) {
	  // hit, then directly update this line in on board ram
	  // set dirty bit
	  wr_req.flag = (rd_res.flag) | 1;
	  wr_req.data = rd_res.data;
	  if (context.wr_req.is_32B) {
	    // update half line
	    if (context.wr_req.is_first_half) {
	      // update first half line
	      wr_req.data.lo = context.wr_req.data.lo;
	    }
	    else {
	      // update last half line
	      wr_req.data.hi = context.wr_req.data.hi;
	    }
	  }
	  should_write_mem_wr_req = true;
	}
	else {
	  // miss, then redirect to pcie
	  should_write_redirect_pcie_wr_req = true;
	  redirectPcieWrReq.data = context.wr_req.data;
	  redirectPcieWrReq.address = get_pcie_address((uchar)(context.wr_req.flag >> 3), context.wr_req.address, slab_start_addr);
	  redirectPcieWrReq.is_32B = context.wr_req.is_32B;
	  redirectPcieWrReq.is_first_half = context.wr_req.is_first_half;
	}
      }
      
      if (should_write_mem_wr_req) {;
	bool dummy = write_channel_nb_altera(mem_wr_req, wr_req);
	assert(dummy);
      }

      if (should_write_redirect_pcie_wr_req) {
	bool dummy = write_channel_nb_altera(redirect_pcie_wr_req_raw, redirectPcieWrReq);
	assert(dummy);
      }
      
    } // end if
    
  } // end while
}
