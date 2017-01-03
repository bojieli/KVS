_CL_VOID
pcie_rx() {
  PCIE_Message msg;
  bool state = false;
  ulong8 signal;
  while (1) {
    bool read_pcie_in;
    msg.raw = read_channel_nb_altera(pcie_in, &read_pcie_in);
    if (read_pcie_in) {
      if (!state) {
	signal.lo = msg.data.data;
      }
      else {
	signal.hi = msg.data.data;
	ClSignal tmp;
	tmp.raw = signal;
	if (tmp.Sig.Cmd == SIGNAL_INIT_SLAB) {
	  bool dummy = write_channel_nb_altera(host_init_slab, signal); // for init signal from host
	  assert(dummy);
	}
	else {
	  bool dummy = write_channel_nb_altera(host_slab_besides_return_req, signal); // for ptr sync signal from host
	  assert(dummy);
	}
      }
      state ^= 1;
    }
  }
}
