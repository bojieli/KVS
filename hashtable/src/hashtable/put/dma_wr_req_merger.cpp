_CL_VOID
hashtable_put_dma_wr_req_merger() {
  ushort inflight_wr_size_left = 0;
  uchar inflight_wr_id = 0;

  DMA_WriteReq_Compressed val_hashtable_put_inline_dma_wr_req;
  bool is_valid_val_hashtable_put_inline_dma_wr_req = false;

  DMA_WriteReq_Compressed val_hashtable_put_offline_update_line_dma_wr_req;
  bool is_valid_val_hashtable_put_offline_update_line_dma_wr_req = false;

  DMA_WriteReq_Compressed val_hashtable_put_offline_update_slab_dma_wr_req;
  bool is_valid_val_hashtable_put_offline_update_slab_dma_wr_req = false;
  
  while (1) {   
    bool read_wr_req = false;
    DMA_WriteReq_Compressed wr_req_compressed;

    if (is_valid_val_hashtable_put_inline_dma_wr_req) {
      read_wr_req = true;
      is_valid_val_hashtable_put_inline_dma_wr_req = false;
      wr_req_compressed = val_hashtable_put_inline_dma_wr_req;
    }
    else if (is_valid_val_hashtable_put_offline_update_line_dma_wr_req) {
      read_wr_req = true;
      is_valid_val_hashtable_put_offline_update_line_dma_wr_req = false;
      wr_req_compressed = val_hashtable_put_offline_update_line_dma_wr_req;
    }
    else if (is_valid_val_hashtable_put_offline_update_slab_dma_wr_req) {
      read_wr_req = true;
      is_valid_val_hashtable_put_offline_update_slab_dma_wr_req = false;
      wr_req_compressed = val_hashtable_put_offline_update_slab_dma_wr_req;
    }
    
    if (!read_wr_req && (inflight_wr_id == 0 || inflight_wr_id == 1)) {
      DMA_WriteReq_Compressed_Double wr_req_compressed_double = read_channel_nb_altera(hashtable_put_inline_dma_wr_req_double, &read_wr_req);
      if (read_wr_req) {
	inflight_wr_id = 1;
	wr_req_compressed = wr_req_compressed_double.x;
	if (wr_req_compressed_double.valid2) {
	  val_hashtable_put_inline_dma_wr_req = wr_req_compressed_double.y;
	  is_valid_val_hashtable_put_inline_dma_wr_req = true;
	}
      }
    }

    if (!read_wr_req && (inflight_wr_id == 0 || inflight_wr_id == 2)) {
      DMA_WriteReq_Compressed_Double wr_req_compressed_double = read_channel_nb_altera(hashtable_put_offline_update_line_dma_wr_req_double, &read_wr_req);
      if (read_wr_req) {
	inflight_wr_id = 2;
	wr_req_compressed = wr_req_compressed_double.x;
	if (wr_req_compressed_double.valid2) {
	  val_hashtable_put_offline_update_line_dma_wr_req = wr_req_compressed_double.y;
	  is_valid_val_hashtable_put_offline_update_line_dma_wr_req = true;
	}
      }
    }

    if (!read_wr_req && (inflight_wr_id == 0 || inflight_wr_id == 3)) {
      DMA_WriteReq_Compressed_Double wr_req_compressed_double = read_channel_nb_altera(hashtable_put_offline_update_slab_dma_wr_req_double, &read_wr_req);
      if (read_wr_req) {
	inflight_wr_id = 3;
	wr_req_compressed = wr_req_compressed_double.x;
	if (wr_req_compressed_double.valid2) {
	  val_hashtable_put_offline_update_slab_dma_wr_req = wr_req_compressed_double.y;
	  is_valid_val_hashtable_put_offline_update_slab_dma_wr_req = true;
	}
      }
    }

    if (!read_wr_req && (inflight_wr_id == 0 || inflight_wr_id == 4)) {
      wr_req_compressed = read_channel_nb_altera(hashtable_put_newline_dma_wr_req, &read_wr_req);
      if (read_wr_req) {
	inflight_wr_id = 4;
      }
    }
      
    if (read_wr_req) {
      if (!inflight_wr_size_left) {
	inflight_wr_size_left = wr_req_compressed.size;
      }

      if (inflight_wr_size_left > 32) {
	inflight_wr_size_left -= 32;
      }
      else {
	inflight_wr_size_left = 0;
	inflight_wr_id = 0;
      }
      
      bool dummy = write_channel_nb_altera(hashtable_put_dma_wr_req, wr_req_compressed);
      assert(dummy);
    }
  }
}
