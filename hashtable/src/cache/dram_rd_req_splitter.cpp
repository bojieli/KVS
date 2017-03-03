_CL_VOID
cache_dram_rd_req_splitter() {
  ushort inflight_rd_req_size = 0;
  ushort inflight_rd_req_addr_offset = 0;

  ulong slab_start_addr = read_channel_altera(init_dram_rd_req_splitter);
  
  while (1) {
    bool read_cache_rd_req;
    DMA_ReadReq_Extended rd_req;
    if (!inflight_rd_req_size) {
      rd_req = read_channel_nb_altera(cache_rd_req, &read_cache_rd_req);
      if (read_cache_rd_req) {
	inflight_rd_req_size = rd_req.size;
      }
      else {
	inflight_rd_req_size = 0;
      }
    }
    
    if (inflight_rd_req_size) {      
      DramReadContext dramReadContext;
      dramReadContext.address = rd_req.address + inflight_rd_req_addr_offset;
      dramReadContext.is_32B = (rd_req.size == 32);
      dramReadContext.should_swapin = rd_req.should_swapin;
      bool dummy = write_channel_nb_altera(dram_rd_req_context, dramReadContext);
      assert(dummy);
      Mem_ReadReq mem_rdreq;
      mem_rdreq.address = get_cache_address(dramReadContext.address, slab_start_addr);
      dummy = write_channel_nb_altera(read_mem_rd_req, mem_rdreq);
      assert(dummy);

      if (inflight_rd_req_size > 64) {
	inflight_rd_req_size -= 64;
	inflight_rd_req_addr_offset += 64;
      }
      else {
	inflight_rd_req_size = 0;
	inflight_rd_req_addr_offset = 0;
      }
    }
  }
}
