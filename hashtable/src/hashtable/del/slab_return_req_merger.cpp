_CL_VOID
hashtable_del_slab_return_req_merger() {
  while (1) {
    bool read_slab_return_req;
    ulong8 val_slab_return_req;

    val_slab_return_req = read_channel_nb_altera(slab_return_req_line, &read_slab_return_req);
    if (!read_slab_return_req) {
      val_slab_return_req = read_channel_nb_altera(slab_return_req_offline_slab, &read_slab_return_req);
    }

    if (read_slab_return_req) {
      bool dummy = write_channel_nb_altera(slab_return_req, val_slab_return_req);
      assert(dummy);
    }
    
  }
}
