_CL_VOID
hashtable_put_line_fetcher() {
  // retreive the line corresponds to the next chain
  ulong2 init = read_channel_altera(init_hashtable_put_line_fetcher);
  ulong slab_start_addr = init.x;
  ulong line_start_addr = init.y;

  ushort inflight_val_size_left = 0;
  uchar inflight_rd_req_id = 0;

  while (1) {
    bool read_put_req = false;
    PutReq put_req;
    
    if (!read_put_req && (inflight_rd_req_id == 0 || inflight_rd_req_id == 1)) {
      put_req = read_channel_nb_altera(return_put_req, &read_put_req);
      if (read_put_req) {
	inflight_rd_req_id = 1;
      }
    }

    if (!read_put_req && (inflight_rd_req_id == 0 || inflight_rd_req_id == 2)) {
      put_req = read_channel_nb_altera(input_put_req, &read_put_req);
      if (read_put_req) {
	inflight_rd_req_id = 2;
      }
    }

    if (!read_put_req && (inflight_rd_req_id == 0 || inflight_rd_req_id == 3)) {
      put_req = read_channel_nb_altera(newline_put_req, &read_put_req);
      if (read_put_req) {
	inflight_rd_req_id = 3;
      }
    }

    if (read_put_req) {
      if (!inflight_val_size_left) {
	inflight_val_size_left = put_req.val_size;
	DMA_ReadReq_Compressed rd_req_compressed;
	if (!put_req.has_last) {
	  rd_req_compressed.address = (put_req.hash1 << 6) + line_start_addr;
	}
	else {
	  rd_req_compressed.address = (put_req.hash1 << 5) + slab_start_addr;	
	}
	rd_req_compressed.size = 64;      
	bool dummy = write_channel_nb_altera(line_fetcher_put_dma_rd_req, rd_req_compressed);
	assert(dummy);
      }
      
      if (inflight_val_size_left > 32) {
	inflight_val_size_left -= 32;
      }
      else {
	inflight_val_size_left = 0;
	inflight_rd_req_id = 0;
      }
      bool dummy = write_channel_nb_altera(fetching_put_req, put_req);
      assert(dummy);
    }
  }  
}


