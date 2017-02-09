_CL_VOID
pcie_tx() {
  ulong8 signal;
  PCIE_Message msg;
  bool state = false, read_signal;
  while (1) {
    msg.data.padbytes = 0;
    msg.data.slot = 32; // signal slot
    msg.data.reserved = 0;
    if (!state) {
      signal = read_channel_nb_altera(slab_besides_return_sync, &read_signal); 
      if (!read_signal) {
	continue;
      }
      msg.data.data = signal.lo;
      msg.data.eop = 0;
    }
    else {
      msg.data.data = signal.hi;
      msg.data.eop = 1;
    }
    bool dummy = write_channel_nb_altera(pcie_out, msg.raw);
    assert(dummy);
    state ^= 1;
  }
}
