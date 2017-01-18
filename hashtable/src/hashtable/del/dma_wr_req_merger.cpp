_CL_VOID
hashtable_del_dma_wr_req_merger() {
  ushort inflight_wr_size_left = 0;
  uchar inflight_wr_id;

  bool is_valid_hashtable_del_dma_wr_req_0 = false;
  bool is_valid_hashtable_del_dma_wr_req_1 = false;
  DMA_WriteReq_Compressed show_ahead_hashtable_del_dma_wr_req_0;
  DMA_WriteReq_Compressed show_ahead_hashtable_del_dma_wr_req_1;

  while (1) {
    if (!is_valid_hashtable_del_dma_wr_req_0) {
      show_ahead_hashtable_del_dma_wr_req_0 =
	read_channel_nb_altera(hashtable_del_dma_wr_req_0, &is_valid_hashtable_del_dma_wr_req_0);
    }

    if (!is_valid_hashtable_del_dma_wr_req_1) {
      show_ahead_hashtable_del_dma_wr_req_1 =
	read_channel_nb_altera(hashtable_del_dma_wr_req_1, &is_valid_hashtable_del_dma_wr_req_1);
    }

    if (!inflight_wr_size_left) {
      if (is_valid_hashtable_del_dma_wr_req_0) {
	inflight_wr_size_left = show_ahead_hashtable_del_dma_wr_req_0.size;
	inflight_wr_id = 0;
      }
      else if (is_valid_hashtable_del_dma_wr_req_1) {
	inflight_wr_size_left = show_ahead_hashtable_del_dma_wr_req_1.size;
	inflight_wr_id = 1;
      }
    }

    bool should_write_hashtable_del_dma_wr_req = false;
    DMA_WriteReq_Compressed wr_req;
    
    if (inflight_wr_size_left) {
      if (inflight_wr_id == 0) {
	assert(is_valid_hashtable_del_dma_wr_req_0);
	is_valid_hashtable_del_dma_wr_req_0 = false;
	wr_req = show_ahead_hashtable_del_dma_wr_req_0;
	should_write_hashtable_del_dma_wr_req = true;
      }
      else if (inflight_wr_id == 1) {
	assert(is_valid_hashtable_del_dma_wr_req_1);
	is_valid_hashtable_del_dma_wr_req_1 = false;
	wr_req = show_ahead_hashtable_del_dma_wr_req_1;
	should_write_hashtable_del_dma_wr_req = true;
      }
      if (inflight_wr_size_left > 32) {
	inflight_wr_size_left -= 32;
      }
      else {
	inflight_wr_size_left = 0;
	inflight_wr_id = 2; // give an invalid number
      }
      assert(should_write_hashtable_del_dma_wr_req);
    }
    
    if (should_write_hashtable_del_dma_wr_req) {
      bool dummy = write_channel_nb_altera(hashtable_del_dma_wr_req, wr_req);
      assert(dummy);
    }
    
  }
}
