_CL_VOID
hashtable_add_dma_wr_req_merger() {
  ushort inflight_wr_size_left = 0;
  uchar inflight_wr_id;

  bool is_valid_hashtable_add_inline_update_line_dma_wr_req_double = false;
  DMA_WriteReq_Compressed_Double show_ahead_hashtable_add_inline_update_line_dma_wr_req_double;
  bool show_ahead_hashtable_add_inline_update_line_dma_wr_req_double_valid1 = true;
  
  bool is_valid_hashtable_add_offline_update_slab_dma_wr_req = false;
  DMA_WriteReq_Compressed show_ahead_hashtable_add_offline_update_slab_dma_wr_req;

  while (1) {
    
    if (!is_valid_hashtable_add_inline_update_line_dma_wr_req_double) {
      show_ahead_hashtable_add_inline_update_line_dma_wr_req_double =
	read_channel_nb_altera(hashtable_add_inline_update_line_dma_wr_req_double,
			       &is_valid_hashtable_add_inline_update_line_dma_wr_req_double);
    }

    if (!is_valid_hashtable_add_offline_update_slab_dma_wr_req) {
      show_ahead_hashtable_add_offline_update_slab_dma_wr_req =
	read_channel_nb_altera(hashtable_add_offline_update_slab_dma_wr_req,
			       &is_valid_hashtable_add_offline_update_slab_dma_wr_req);
    }

    if (!inflight_wr_size_left) {
      if (is_valid_hashtable_add_inline_update_line_dma_wr_req_double) {
	inflight_wr_size_left = show_ahead_hashtable_add_inline_update_line_dma_wr_req_double.x.size;
	inflight_wr_id = 0;
      }
      else if (is_valid_hashtable_add_offline_update_slab_dma_wr_req) {
	inflight_wr_size_left = show_ahead_hashtable_add_offline_update_slab_dma_wr_req.size;
	inflight_wr_id = 1;
      }
    }

    bool should_write_hashtable_add_dma_wr_req = false;

    DMA_WriteReq_Compressed wr_req_compressed;
    if (inflight_wr_size_left) {
      if (inflight_wr_id == 0) {
	if (show_ahead_hashtable_add_inline_update_line_dma_wr_req_double_valid1) {
	  show_ahead_hashtable_add_inline_update_line_dma_wr_req_double_valid1 = false;
	  wr_req_compressed = show_ahead_hashtable_add_inline_update_line_dma_wr_req_double.x;
	  should_write_hashtable_add_dma_wr_req = true;
	  if (!show_ahead_hashtable_add_inline_update_line_dma_wr_req_double.valid2) {
	    is_valid_hashtable_add_inline_update_line_dma_wr_req_double = false;
	  }
	}
	else {
	  is_valid_hashtable_add_inline_update_line_dma_wr_req_double = false;
	  wr_req_compressed = show_ahead_hashtable_add_inline_update_line_dma_wr_req_double.y;
	  should_write_hashtable_add_dma_wr_req = true;	  
	}
      }
      else if (inflight_wr_id == 1) {
	is_valid_hashtable_add_offline_update_slab_dma_wr_req = false;
        wr_req_compressed = show_ahead_hashtable_add_offline_update_slab_dma_wr_req;
	should_write_hashtable_add_dma_wr_req = true;
      }
      assert(should_write_hashtable_add_dma_wr_req);
      
      if (inflight_wr_size_left > 32) {
	inflight_wr_size_left -= 32;
      }
      else {
	inflight_wr_size_left = 0;
      }
    }
    
    if (should_write_hashtable_add_dma_wr_req) {
      bool dummy = write_channel_nb_altera(hashtable_add_dma_wr_req, wr_req_compressed);
      assert(dummy);
    }
  }
}
