_CL_VOID
hashtable_get_res_merger() {
  ushort inflight_get_res_size_left = 0;
  uchar inflight_get_res_id = 0;
  
  bool has_inflight_res = false;
  bool is_first = true;
  
  while (1) {
    bool should_write_get_res = false;
    GetRes val_write_get_res;

    bool should_write_array_get_req_info = false;
    ArrayGetReqInfo val_write_array_get_req_info;

    bool read_get_res = false;
    GetRes get_res;

    if (!read_get_res && (inflight_get_res_id == 0 || inflight_get_res_id == 1)) {
      get_res = read_channel_nb_altera(get_inline_res, &read_get_res);
      if (read_get_res) {
	inflight_get_res_id = 1;
      }
    }

    if (!read_get_res && (inflight_get_res_id == 0 || inflight_get_res_id == 2)) {
      get_res = read_channel_nb_altera(get_offline_res, &read_get_res);
      if (read_get_res) {
	inflight_get_res_id = 2;
      }
    }

    if (read_get_res) {
      if (!has_inflight_res) {
	has_inflight_res = true;
	inflight_get_res_size_left = get_res.val_size;
      }
      
      if (inflight_get_res_size_left > 32) {
	inflight_get_res_size_left -= 32;
	if (is_first) {
	  is_first = false;
	  if (get_res.is_array_first) {
	    ArrayGetReqInfo info;
	    info.net_meta = get_res.net_meta;
	    info.key = get_res.key;
	    info.key_size = get_res.key_size;
	    info.cnt = (get_res.val.x >> 48) - 1;
	    
	    if (info.cnt) {
	      should_write_array_get_req_info = true;
	      val_write_array_get_req_info = info;
	    }
	  }
	}	  
      }
      else {
	inflight_get_res_size_left = 0;
	inflight_get_res_id = 0;
	has_inflight_res = false;
	is_first = true;
      }

      write_channel_altera(output_get_res, get_res);

      if (should_write_array_get_req_info) {
	bool dummy = write_channel_nb_altera(array_get_req_info, val_write_array_get_req_info);
	assert(dummy);
      }
      
    }
  }
}

