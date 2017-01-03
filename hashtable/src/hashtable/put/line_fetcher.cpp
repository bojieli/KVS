_CL_VOID
hashtable_put_line_fetcher() {
  // retreive the line corresponds to the next chain
  ulong2 init = read_channel_altera(init_hashtable_put_line_fetcher);
  ulong slab_start_addr = init.x;
  ulong line_start_addr = init.y;

  bool is_valid_return_put_req = false;
  PutReq show_ahead_return_put_req;

  bool is_valid_input_put_req = false;
  PutReq show_ahead_input_put_req;

  bool is_valid_newline_put_req = false;
  PutReq show_ahead_newline_put_req;

  ushort inflight_val_size_left = 0;
  uchar inflight_rd_req_id = 0;

  while (1) {
    if (!is_valid_return_put_req) {
      show_ahead_return_put_req = read_channel_nb_altera(return_put_req, &is_valid_return_put_req);
    }

    if (!is_valid_input_put_req) {
      show_ahead_input_put_req = read_channel_nb_altera(input_put_req, &is_valid_input_put_req);
    }

    if (!is_valid_newline_put_req) {
      show_ahead_newline_put_req = read_channel_nb_altera(newline_put_req, &is_valid_newline_put_req);
    }

    if (!inflight_val_size_left) {
      bool has_req = false;
      PutReq req;
      if (is_valid_return_put_req) {
	req = show_ahead_return_put_req;
	inflight_val_size_left = req.val_size;
	inflight_rd_req_id = 0;
	has_req = true;
      }
      else if (is_valid_input_put_req) {
	req = show_ahead_input_put_req;
	inflight_val_size_left = req.val_size;
	inflight_rd_req_id = 1;
	has_req = true;
      }
      else if (is_valid_newline_put_req) {
	req = show_ahead_newline_put_req;
	inflight_val_size_left = req.val_size;
	inflight_rd_req_id = 2;
	has_req = true;
      }
      
      if (has_req) {
	DMA_ReadReq rd_req;
	if (!req.has_last) {
	  rd_req.req.address = (req.hash1 << 6) + line_start_addr;
	}
	else {
	  rd_req.req.address = (req.hash1 << 5) + slab_start_addr;	
	}
	rd_req.req.size = 64;      
	write_channel_altera(line_fetcher_put_dma_rd_req, rd_req.raw);
      }
    }

    if (inflight_val_size_left) {
      PutReq req;
      if (inflight_rd_req_id == 0) {
	assert(is_valid_return_put_req);
	is_valid_return_put_req = false;
	req = show_ahead_return_put_req;
      }
      else if (inflight_rd_req_id == 1) {
	assert(is_valid_input_put_req);
	is_valid_input_put_req = false;
	req = show_ahead_input_put_req;
      }
      else if (inflight_rd_req_id == 2) {
	assert(is_valid_newline_put_req);
	is_valid_newline_put_req = false;
	req = show_ahead_newline_put_req;
      }

      if (inflight_val_size_left > 32) {
	inflight_val_size_left -= 32;
      }
      else {
	inflight_val_size_left = 0;
      }
      bool dummy = write_channel_nb_altera(fetching_put_req, req);
      assert(dummy);
    }    
  }  
}
