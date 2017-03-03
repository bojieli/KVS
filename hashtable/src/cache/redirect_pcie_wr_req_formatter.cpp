_CL_VOID
cache_redirect_pcie_wr_req_formatter() {
  bool is_valid_redirect_raw = false;
  RedirectPcieWrReqRaw show_ahead_redirect_raw;

  bool state = false;
    
  while (1) {
    if (!is_valid_redirect_raw) {
      show_ahead_redirect_raw = read_channel_nb_altera(redirect_pcie_wr_req_raw, &is_valid_redirect_raw);
    }

    if (is_valid_redirect_raw) {
      DMA_WriteReq wr_req;
      RedirectPcieWrReqRaw redirect_raw = show_ahead_redirect_raw;

      if (!state) {
	if (redirect_raw.is_32B) {
	  if (redirect_raw.is_first_half) {
	    wr_req.req.address = redirect_raw.address;
	    wr_req.req.size = 32;
	    wr_req.req.data = redirect_raw.data.lo;
	    is_valid_redirect_raw = false;
	  }
	  else {
	    wr_req.req.address = redirect_raw.address + 32;
	    wr_req.req.size = 32;
	    wr_req.req.data = redirect_raw.data.hi;
	    is_valid_redirect_raw = false;
	  }
	}
	else {
	  state = true;
	  wr_req.req.address = redirect_raw.address;
	  wr_req.req.size = 64;
	  wr_req.req.data = redirect_raw.data.lo;
	}
      }
      else {
	state = false;
	wr_req.req.data = redirect_raw.data.hi;
	is_valid_redirect_raw = false;
      }

      bool dummy = write_channel_nb_altera(redirect_pcie_wr_req, wr_req);
      assert(dummy);
      
    }

  }
}
