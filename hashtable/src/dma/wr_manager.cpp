// multiplex dma wr channels
_CL_VOID
dma_wr_manager() {
  ushort inflight_wr_req_left_size = 0;
  uchar inflight_wr_req_id;
  
  bool is_valid_slab_dma_wr_req = false;
  ulong8 show_ahead_slab_dma_wr_req;

  bool is_valid_hashtable_del_dma_wr_req = false;
  ulong8 show_ahead_hashtable_del_dma_wr_req;

  bool is_valid_hashtable_put_dma_wr_req = false;
  ulong8 show_ahead_hashtable_put_dma_wr_req;
			 
  while (1) {
    
    bool should_write_dma_wr_req = false;
    ulong8 val_write_dma_wr_req;
  
    if (!is_valid_slab_dma_wr_req) {
      show_ahead_slab_dma_wr_req = read_channel_nb_altera(slab_dma_wr_req, &is_valid_slab_dma_wr_req);
    }
    
    if (!is_valid_hashtable_del_dma_wr_req) {
      show_ahead_hashtable_del_dma_wr_req = read_channel_nb_altera(hashtable_del_dma_wr_req, &is_valid_hashtable_del_dma_wr_req);
    }

    if (!is_valid_hashtable_put_dma_wr_req) {
      show_ahead_hashtable_put_dma_wr_req = read_channel_nb_altera(hashtable_put_dma_wr_req, &is_valid_hashtable_put_dma_wr_req);
    }

    if (!inflight_wr_req_left_size) {
      if (is_valid_slab_dma_wr_req) {
	DMA_ReadReq tmp;
	tmp.raw = show_ahead_slab_dma_wr_req;
	inflight_wr_req_left_size = tmp.req.size;
	assert(tmp.req.size > 0);
	inflight_wr_req_id = 1;
      }
      else if (is_valid_hashtable_del_dma_wr_req) {
	DMA_ReadReq tmp;
	tmp.raw = show_ahead_hashtable_del_dma_wr_req;
	inflight_wr_req_left_size = tmp.req.size;
	assert(tmp.req.size > 0);
	inflight_wr_req_id = 2;
      }
      else if (is_valid_hashtable_put_dma_wr_req) {
	DMA_ReadReq tmp;
	tmp.raw = show_ahead_hashtable_put_dma_wr_req;
	inflight_wr_req_left_size = tmp.req.size;
	assert(tmp.req.size > 0);
	inflight_wr_req_id = 3;
      }
      else {
	// other write channels
      }
    }

    if (inflight_wr_req_left_size) {
      assert(
	     inflight_wr_req_id == 1 ||
	     inflight_wr_req_id == 2 ||
	     inflight_wr_req_id == 3
	     );
      if (inflight_wr_req_id == 1) {
	if (is_valid_slab_dma_wr_req) {
	  is_valid_slab_dma_wr_req = 0;
	  if (inflight_wr_req_left_size > 32) {
	    inflight_wr_req_left_size -= 32;
	  }
	  else {
	    inflight_wr_req_left_size = 0;
	  }
	  should_write_dma_wr_req = true;
	  val_write_dma_wr_req = show_ahead_slab_dma_wr_req;
	}
      }
      else if (inflight_wr_req_id == 2) {
	if (is_valid_hashtable_del_dma_wr_req) {
	  is_valid_hashtable_del_dma_wr_req = 0;
	  if (inflight_wr_req_left_size > 32) {
	    inflight_wr_req_left_size -= 32;
	  }
	  else {
	    inflight_wr_req_left_size = 0;
	  }
	  should_write_dma_wr_req = true;
	  val_write_dma_wr_req = show_ahead_hashtable_del_dma_wr_req;
	}	
      }
      else if (inflight_wr_req_id == 3) {
	if (is_valid_hashtable_put_dma_wr_req) {
	  is_valid_hashtable_put_dma_wr_req = 0;
	  if (inflight_wr_req_left_size > 32) {
	    inflight_wr_req_left_size -= 32;
	  }
	  else {
	    inflight_wr_req_left_size = 0;
	  }
	  should_write_dma_wr_req = true;
	  val_write_dma_wr_req = show_ahead_hashtable_put_dma_wr_req;;
	}	
      }
      else {
	// other channels
      }

      if (should_write_dma_wr_req) {	
	bool dummy = write_channel_nb_altera(dma_wr_req, val_write_dma_wr_req);
      }
    }
    
  }
}
