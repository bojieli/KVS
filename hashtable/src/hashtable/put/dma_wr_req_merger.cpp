_CL_VOID
hashtable_put_dma_wr_req_merger() {
  ushort inflight_wr_size_left = 0;
  uchar inflight_wr_id;

  bool is_valid_hashtable_put_inline_dma_wr_req_double = false;
  Ulong16 show_ahead_hashtable_put_inline_dma_wr_req_double;
  bool show_ahead_hashtable_put_inline_dma_wr_req_double_valid1 = true;
  
  bool is_valid_hashtable_put_offline_update_line_dma_wr_req_double = false;
  Ulong16 show_ahead_hashtable_put_offline_update_line_dma_wr_req_double;
  bool show_ahead_hashtable_put_offline_update_line_dma_wr_req_double_valid1 = true;
  
  bool is_valid_hashtable_put_offline_update_slab_dma_wr_req_double = false;
  Ulong16 show_ahead_hashtable_put_offline_update_slab_dma_wr_req_double;
  bool show_ahead_hashtable_put_offline_update_slab_dma_wr_req_double_valid1 = true;  
    
  bool is_valid_hashtable_put_newline_dma_wr_req = false;  
  ulong8 show_ahead_hashtable_put_newline_dma_wr_req;
  
  while (1) {

    if (!is_valid_hashtable_put_inline_dma_wr_req_double) {
      show_ahead_hashtable_put_inline_dma_wr_req_double =
	read_channel_nb_altera(hashtable_put_inline_dma_wr_req_double,
			       &is_valid_hashtable_put_inline_dma_wr_req_double);
      show_ahead_hashtable_put_inline_dma_wr_req_double_valid1 = true;
    }

    if (!is_valid_hashtable_put_offline_update_line_dma_wr_req_double) {
      show_ahead_hashtable_put_offline_update_line_dma_wr_req_double =
	read_channel_nb_altera(hashtable_put_offline_update_line_dma_wr_req_double,
			       &is_valid_hashtable_put_offline_update_line_dma_wr_req_double);
      show_ahead_hashtable_put_offline_update_line_dma_wr_req_double_valid1 = true;
    }

    if (!is_valid_hashtable_put_offline_update_slab_dma_wr_req_double) {
      show_ahead_hashtable_put_offline_update_slab_dma_wr_req_double =
	read_channel_nb_altera(hashtable_put_offline_update_slab_dma_wr_req_double,
			       &is_valid_hashtable_put_offline_update_slab_dma_wr_req_double);
      show_ahead_hashtable_put_offline_update_slab_dma_wr_req_double_valid1 = true;
    }

    if (!is_valid_hashtable_put_newline_dma_wr_req) {
      show_ahead_hashtable_put_newline_dma_wr_req =
	read_channel_nb_altera(hashtable_put_newline_dma_wr_req,
			       &is_valid_hashtable_put_newline_dma_wr_req);
    }
    
    if (!inflight_wr_size_left) {
      if (is_valid_hashtable_put_inline_dma_wr_req_double) {
	DMA_WriteReq tmp;
	tmp.raw = show_ahead_hashtable_put_inline_dma_wr_req_double.x;
	inflight_wr_size_left = tmp.req.size;
	inflight_wr_id = 0;
	// cout << "enter 0........." << endl;
      }
      else if (is_valid_hashtable_put_offline_update_line_dma_wr_req_double) {
	DMA_WriteReq tmp;
	tmp.raw = show_ahead_hashtable_put_offline_update_line_dma_wr_req_double.x;
	inflight_wr_size_left = tmp.req.size;
	inflight_wr_id = 1;
	// cout << "enter 1........." << endl;
	// cout << dec << inflight_wr_size_left << endl;
      }
      else if (is_valid_hashtable_put_offline_update_slab_dma_wr_req_double) {
	DMA_WriteReq tmp;
	tmp.raw = show_ahead_hashtable_put_offline_update_slab_dma_wr_req_double.x;
	inflight_wr_size_left = tmp.req.size;
	inflight_wr_id = 2;
	// cout << "enter 2........." << endl;
	// cout << dec << inflight_wr_size_left << endl;
      }
      else if (is_valid_hashtable_put_newline_dma_wr_req) {
	DMA_WriteReq tmp;
	tmp.raw = show_ahead_hashtable_put_newline_dma_wr_req;
	inflight_wr_size_left = tmp.req.size;
	inflight_wr_id = 3;
	// cout << "enter 3........." << endl;
      }
    }

    bool should_write_hashtable_put_dma_wr_req = false;

    DMA_WriteReq wr_req;
    if (inflight_wr_size_left) {
      if (inflight_wr_id == 0) {
	if (show_ahead_hashtable_put_inline_dma_wr_req_double_valid1) {
	  show_ahead_hashtable_put_inline_dma_wr_req_double_valid1 = false;
	  wr_req.raw = show_ahead_hashtable_put_inline_dma_wr_req_double.x;
	  should_write_hashtable_put_dma_wr_req = true;
	  if (!show_ahead_hashtable_put_inline_dma_wr_req_double.valid2) {
	    is_valid_hashtable_put_inline_dma_wr_req_double = false;
	  }
	}
	else {
	  is_valid_hashtable_put_inline_dma_wr_req_double = false;
	  wr_req.raw = show_ahead_hashtable_put_inline_dma_wr_req_double.y;
	  should_write_hashtable_put_dma_wr_req = true;	  
	}
      }
      else if (inflight_wr_id == 1) {
	if (show_ahead_hashtable_put_offline_update_line_dma_wr_req_double_valid1) {
	  show_ahead_hashtable_put_offline_update_line_dma_wr_req_double_valid1 = false;
	  wr_req.raw = show_ahead_hashtable_put_offline_update_line_dma_wr_req_double.x;
	  should_write_hashtable_put_dma_wr_req = true;
	  if (!show_ahead_hashtable_put_offline_update_line_dma_wr_req_double.valid2) {
	    is_valid_hashtable_put_offline_update_line_dma_wr_req_double = false;
	  }
	}
	else {
	  is_valid_hashtable_put_offline_update_line_dma_wr_req_double = false;
	  wr_req.raw = show_ahead_hashtable_put_offline_update_line_dma_wr_req_double.y;
	  should_write_hashtable_put_dma_wr_req = true;	  
	}
      }
      else if (inflight_wr_id == 2) {
	if (show_ahead_hashtable_put_offline_update_slab_dma_wr_req_double_valid1) {
	  show_ahead_hashtable_put_offline_update_slab_dma_wr_req_double_valid1 = false;
	  wr_req.raw = show_ahead_hashtable_put_offline_update_slab_dma_wr_req_double.x;
	  should_write_hashtable_put_dma_wr_req = true;
	  if (!show_ahead_hashtable_put_offline_update_slab_dma_wr_req_double.valid2) {
	    is_valid_hashtable_put_offline_update_slab_dma_wr_req_double = false;
	  }
	}
	else {
	  is_valid_hashtable_put_offline_update_slab_dma_wr_req_double = false;
	  wr_req.raw = show_ahead_hashtable_put_offline_update_slab_dma_wr_req_double.y;
	  should_write_hashtable_put_dma_wr_req = true;	  
	}      
      }
      else if (inflight_wr_id == 3) {
	is_valid_hashtable_put_newline_dma_wr_req = false;
	wr_req.raw = show_ahead_hashtable_put_newline_dma_wr_req;
	should_write_hashtable_put_dma_wr_req = true;
      }      

      assert(should_write_hashtable_put_dma_wr_req);
      
      if (inflight_wr_size_left > 32) {
	inflight_wr_size_left -= 32;
      }
      else {
	inflight_wr_size_left = 0;
      }
    }
    
    if (should_write_hashtable_put_dma_wr_req) {
      bool dummy = write_channel_nb_altera(hashtable_put_dma_wr_req, wr_req.raw);
      assert(dummy);
    }
  }
}
