_CL_VOID
slab_dma_rd_req_merger() {
  while (1) {
    bool read_slab_dma_rd_req, dummy;
    DmaReadReqWithId dmaReadReqWithId = read_channel_nb_altera(slab_init_dma_rd_req_with_id, &read_slab_dma_rd_req);

    if (!read_slab_dma_rd_req) {
      dmaReadReqWithId = read_channel_nb_altera(slab_non_init_dma_rd_req_with_id, &read_slab_dma_rd_req);
    }

    if (read_slab_dma_rd_req) {
      dummy = write_channel_nb_altera(slab_dma_rd_req, dmaReadReqWithId.req.raw);
      DmaContext context;
      context.id = dmaReadReqWithId.id;
      context.size = (ushort)dmaReadReqWithId.req.req.size;
      dummy = write_channel_nb_altera(slab_dma_handler_rd_req_context, context);
    }
  }
}
