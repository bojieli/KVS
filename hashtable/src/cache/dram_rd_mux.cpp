_CL_VOID
cache_dram_rd_mux() {
  bool is_valid_mem_rd_res = false;
  Mem_ReadRes show_ahead_mem_rd_res;
  bool is_valid_mem_rd_req_context = false;
  bool show_ahead_mem_rd_req_context;
  
  while (1) {
    bool read_rd_req;
    bool is_read_mem_rd_req;
    Mem_ReadReq mem_rdreq;
    mem_rdreq = read_channel_nb_altera(read_mem_rd_req, &read_rd_req);
    if (!read_rd_req) {
      mem_rdreq = read_channel_nb_altera(write_mem_rd_req, &read_rd_req);
      if (read_rd_req) {
	is_read_mem_rd_req = false;
      }
    }
    else {
      is_read_mem_rd_req = true;
    }
    
    if (read_rd_req) {
      bool dummy = write_channel_nb_altera(mem_rd_req_context, is_read_mem_rd_req);
      assert(dummy);
      dummy = write_channel_nb_altera(mem_rd_req, mem_rdreq);
      assert(dummy);
    }
    
    if (!is_valid_mem_rd_res) {
      show_ahead_mem_rd_res = read_channel_nb_altera(mem_rd_res, &is_valid_mem_rd_res);
    }

    if (!is_valid_mem_rd_req_context) {
      show_ahead_mem_rd_req_context = read_channel_nb_altera(mem_rd_req_context, &is_valid_mem_rd_req_context);
    }

    if (is_valid_mem_rd_res && is_valid_mem_rd_req_context) {
      is_valid_mem_rd_res = false;
      is_valid_mem_rd_req_context = false;
      Mem_ReadRes mem_rdres = show_ahead_mem_rd_res;
      
      bool context = show_ahead_mem_rd_req_context;
      if (context) {
	// -> read_mem_rd_res
	bool dummy = write_channel_nb_altera(read_mem_rd_res, mem_rdres);
	assert(dummy); 
      }
      else {
	// -> write_mem_rd_res
	bool dummy = write_channel_nb_altera(write_mem_rd_res, mem_rdres);
	assert(dummy); 	
      }
    }
        
  }
}

