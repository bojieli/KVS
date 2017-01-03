_CL_VOID
hashtable_put_res_merger() {
  while (1) {
    bool read_put_res;
    PutRes val_put_res;
    val_put_res = read_channel_nb_altera(put_inline_res, &read_put_res);
    if (!read_put_res) {
      val_put_res = read_channel_nb_altera(put_offline_res, &read_put_res);
    }
    if (read_put_res) {
      bool dummy = write_channel_nb_altera(output_put_res, val_put_res);
      assert(dummy);
    }
  }
}
