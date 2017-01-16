_CL_VOID
hashtable_add_line_fetcher() {
  ulong2 init = read_channel_altera(init_hashtable_add_line_fetcher);
  ulong slab_start_addr = init.x;
  ulong line_start_addr = init.y;
  
  while (1) {
    bool read_add_req;
    AddReq req;
    bool read_from_input;
    req = read_channel_nb_altera(array_add_req, &read_add_req);
    if (!read_add_req) {
      req = read_channel_nb_altera(return_add_req, &read_add_req);
      if (!read_add_req) {
	req = read_channel_nb_altera(input_add_req, &read_add_req);
	if (!read_add_req) {
	  // ...
	}
	else {
	  // cout << "input " <<  dec << (uint)req.delta << endl;
	  read_from_input = true;
	}
      }
      else {
	// cout << "return " << dec << (uint)req.delta << endl;
	read_from_input = false;
      }
    }
    else {
      // cout << "array " <<  dec << (uint)req.delta << endl;
      req.is_array = true;
      read_from_input = true;
    }
    
    if (read_add_req) {
      DMA_ReadReq rd_req;
      if (read_from_input) {
	rd_req.req.address = (req.hash1 << 6) + line_start_addr;
	req.has_last = false;
      }
      else {
	rd_req.req.address = (req.hash1 << 5) + slab_start_addr;
	req.has_last = true;
      }
      rd_req.req.size = 64;
      write_channel_altera(line_fetcher_add_dma_rd_req, rd_req.raw);
      write_channel_altera(fetching_add_req, req);
    }
  }
}
