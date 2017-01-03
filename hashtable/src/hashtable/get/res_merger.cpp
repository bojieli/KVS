_CL_VOID
hashtable_get_res_merger() {
  ushort inflight_get_res_size_left = 0;
  uchar inflight_get_res_id = 0;
  
  bool is_valid_get_inline_res = false;
  bool is_valid_get_offline_res = false;
  GetRes show_ahead_get_inline_res;
  GetRes show_ahead_get_offline_res;

  bool has_inflight_res = false;
  
  while (1) {
    bool should_write_get_res = false;
    GetRes val_write_get_res;

    if (!is_valid_get_inline_res) {
      show_ahead_get_inline_res = read_channel_nb_altera(get_inline_res, &is_valid_get_inline_res);
    }

    if (!is_valid_get_offline_res) {
      show_ahead_get_offline_res = read_channel_nb_altera(get_offline_res, &is_valid_get_offline_res);
    }
    
    if (!has_inflight_res) {
      if (is_valid_get_inline_res) {
	inflight_get_res_size_left = show_ahead_get_inline_res.val_size;
	inflight_get_res_id = 1;
	has_inflight_res = true;
      }
      else if (is_valid_get_offline_res) {
	inflight_get_res_size_left = show_ahead_get_offline_res.val_size;
	inflight_get_res_id = 2;
	has_inflight_res = true;
      }
    }

    if (has_inflight_res) {
      if (inflight_get_res_id == 1 && is_valid_get_inline_res) {
	is_valid_get_inline_res = false;
	val_write_get_res = show_ahead_get_inline_res;
	should_write_get_res = true;
      }
      else if (inflight_get_res_id == 2 && is_valid_get_offline_res) {
	is_valid_get_offline_res = false;
	val_write_get_res = show_ahead_get_offline_res;
	should_write_get_res = true;
      }
      if (should_write_get_res) {
	if (inflight_get_res_size_left > 32) {
	  inflight_get_res_size_left -= 32;
	}
	else {
	  inflight_get_res_size_left = 0;
	  has_inflight_res = false;
	}
      }
    }
    
    if (should_write_get_res) {
      write_channel_altera(output_get_res, val_write_get_res);
    }
  }
}
