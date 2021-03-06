_CL_VOID
slab_besides_return_req_merger() {
  while (1) {
    uchar req_id;
    bool read_slab_besides_return_req;
    // pointer synchronization
    SlabRequest val_slab_besides_return_req = read_channel_nb_altera(host_slab_besides_return_req, &read_slab_besides_return_req);
    if (!read_slab_besides_return_req) {
      // slab request
      val_slab_besides_return_req = read_channel_nb_altera(hashtable_put_offline_slab_req, &read_slab_besides_return_req);
      if (!read_slab_besides_return_req) {
	// new line request
	val_slab_besides_return_req = read_channel_nb_altera(hashtable_put_newline_slab_req, &read_slab_besides_return_req);
	if (!read_slab_besides_return_req) {
	  // other requests
	}
	else {
	  val_slab_besides_return_req.cmd = SIGNAL_REQUEST;
	  req_id = 2;
	}
      }
      else {
	val_slab_besides_return_req.cmd = SIGNAL_REQUEST;
	req_id = 1;
      }
    }
    else {
      req_id = 0;
    }
    if (read_slab_besides_return_req) {
      bool dummy;
      #ifdef _CSIM
      write_channel_altera(slab_besides_return_req, val_slab_besides_return_req);
      #else
      dummy = write_channel_nb_altera(slab_besides_return_req, val_slab_besides_return_req);
      #endif
      if (req_id != 0) {
	dummy = write_channel_nb_altera(slab_besides_return_req_context, req_id);
	assert(dummy);
      }
    }

#ifndef _UNIT_TEST_SLAB
    bool read_slab_besides_return_res;
    ulong val_slab_besides_return_res;
    val_slab_besides_return_res = read_channel_nb_altera(slab_besides_return_res, &read_slab_besides_return_res);
    if (read_slab_besides_return_res) {
      bool dummy;
      uchar res_id;
      res_id = read_channel_nb_altera(slab_besides_return_req_context, &dummy);
      assert(dummy);
      if (res_id == 1) {
	dummy = write_channel_nb_altera(slab_besides_return_res_offline, val_slab_besides_return_res);
	assert(dummy);
      }
      else if (res_id == 2) {
	dummy = write_channel_nb_altera(slab_besides_return_res_newline, val_slab_besides_return_res);
	assert(dummy);
      }
    }
#endif    
  }
}
