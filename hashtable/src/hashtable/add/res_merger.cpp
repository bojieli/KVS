_CL_VOID
hashtable_add_res_merger() {
  while (1) {
    bool read_add_res;
    AddRes val_add_res;
    val_add_res = read_channel_nb_altera(add_inline_res, &read_add_res);
    if (!read_add_res) {
      val_add_res = read_channel_nb_altera(add_offline_res, &read_add_res);
    }
    if (read_add_res) {
      bool dummy = write_channel_nb_altera(output_add_res, val_add_res);
      assert(dummy);
    }
  }
}
