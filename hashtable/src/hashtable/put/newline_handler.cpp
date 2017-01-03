_CL_VOID
hashtable_put_newline_handler() {
  // 1. get new line address
  // 2. update the data field old line chain
  // 3. redirect the request to fetching_put_req

  ulong2 init_data = read_channel_altera(init_hashtable_put_newline_handler);
  ulong slab_start_addr = init_data.x;
  ulong line_start_addr = init_data.y;
  
  bool got_newline = false;
  
  bool is_valid_newline_addr = false;
  ulong show_ahead_newline_addr;
  
  bool is_valid_put_newline_handler = false;
  PutNewlineType show_ahead_put_newline_handler;
  
  ushort val_size_left = 0;  
  
  while (1) {
    if (!is_valid_newline_addr) {
      show_ahead_newline_addr = read_channel_nb_altera(slab_besides_return_res_newline, &is_valid_newline_addr);
    }

    if (!is_valid_put_newline_handler) {
      show_ahead_put_newline_handler = read_channel_nb_altera(put_newline_handler, &is_valid_put_newline_handler);
    }

    PutNewlineType put_newline_data = show_ahead_put_newline_handler;	
    
    if (!got_newline) {
      if (is_valid_newline_addr && is_valid_put_newline_handler) {
	got_newline = true;
	is_valid_newline_addr = false;
	val_size_left = put_newline_data.req.val_size;
	ulong line_addr = show_ahead_newline_addr;
	DMA_WriteReq wr_req;
	if (!put_newline_data.req.has_last) {
	  wr_req.req.address = (put_newline_data.req.hash1 << 6) + line_start_addr + 32;
	}
	else {
	  wr_req.req.address = (put_newline_data.req.hash1 << 5) + slab_start_addr + 32;
	}
	wr_req.req.size = 32;
	wr_req.req.data = put_newline_data.half_line;
	wr_req.req.data.w = ((wr_req.req.data.w >> 32) << 32) | (((line_addr - slab_start_addr) >> 5) << 2) | (1);
	bool dummy;
	dummy = write_channel_nb_altera(hashtable_put_newline_dma_wr_req, wr_req.raw);
	assert(dummy);
	put_newline_data.req.has_last = true;
	put_newline_data.req.hash1 = (line_addr - slab_start_addr) >> 5;
      }
    }
    
    if (got_newline) {
      is_valid_put_newline_handler = false;
      if (val_size_left > 32) {
	val_size_left -= 32;
      }
      else {
	val_size_left = 0;
	got_newline = false;
      }
      bool dummy;
      dummy = write_channel_nb_altera(newline_put_req, put_newline_data.req);
      assert(dummy);
    }
  }
}
