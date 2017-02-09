// hashtable/put/comparator_pipeline_two -> hashtable/put/newline_handler
decl_channel(PutNewlineType, 128, put_newline_handler);
// slab/dma_rd_req_merger -> dma/rd_manager
decl_channel(DMA_ReadReq_Compressed, 128, slab_dma_rd_req);
// dma/rd_manager -> dma/rd_manager
decl_channel(DmaContext, 128, dma_manager_rd_req_context);
// slab/dma_rd_handler -> slab/dma_rd_req_merger
decl_channel(DmaReadReqWithId, 5, slab_init_dma_rd_req_with_id);
// slab/dma_rd_handler -> slab/dma_rd_req_merger
decl_channel(DmaReadReqWithId, 128, slab_non_init_dma_rd_req_with_id);
// slab/dma_rd_req_merger -> slab/dma_rd_handler
decl_channel(DmaContext, 128, slab_dma_handler_rd_req_context);
// hashtable/line_fetcher/line_fetcher_rd_handler -> dma/rd_manager
decl_channel(DMA_ReadReq_Compressed, 128, line_fetcher_dma_rd_req);
// dma/rd_manager -> dma/rd_mux
decl_channel(ulong8, 256, dma_rd_req);
// dma/rd_demux -> dma/rd_manager
#ifdef _CSIM
decl_channel(ulong4, 4096, dma_rd_res);
#else
decl_channel(ulong4, 256, dma_rd_res);
#endif
// dma/wr_manager -> dma/wr_mux
decl_channel(ulong8, 256, dma_wr_req);
// fpga -> host
decl_channel(ulong8, 256, dma1_rd_req);
// host -> fpga
decl_channel(ulong4, 256, dma1_rd_res);
// fpga -> host
decl_channel(ulong8, 256, dma1_wr_req);
// fpga -> host
decl_channel(ulong8, 256, dma0_rd_req);
// host -> fpga
decl_channel(ulong4, 256, dma0_rd_res);
// fpga -> host
decl_channel(ulong8, 256, dma0_wr_req);
// dma/rd_mux -> dma/rd_demux
decl_channel(ushort, 256, dma_rd_req_size);
// slab/dma_wr_handler -> dma/wr_manager   
decl_channel(DMA_WriteReq_Compressed, 512, slab_dma_wr_req);
// slab/besides_return -> slab/dma_wr_handler
decl_channel(DMA_WriteReq_Compressed, 128, slab_bin_dma_wr_req[SLAB_BIN_COUNT]);
// hashtable/src/dma/rd_manager -> hashtable/line_fetcher/line_fetcher_rd_handler
decl_channel(ulong4, 128, line_fetcher_dma_rd_res);
// dma/rd_manager -> slab/dma_rd_handler
#ifdef _CSIM
decl_channel(ulong4, 4096, slab_dma_rd_res);
#else
decl_channel(ulong4, 256, slab_dma_rd_res);
#endif
// slab/besides_return -> slab/dma_rd_handler
decl_channel(ulong8, 1, init_slab_dma_rd_handler);
// pcie/rx -> slab/besides_return
decl_channel(ulong8, 1, host_init_slab);
// slab/dma_rd_handler -> slab/besides_return
decl_channel(ulong4, 512, slab_cache_table[SLAB_BIN_COUNT]);
// src/slab/return -> src/slab/besides_return
decl_channel(ulong4, 256, slab_return_table[SLAB_BIN_COUNT]);
// slab/dma_rd_handler -> slab/besides_return
decl_channel(bool, 1, slab_init_finish);
// slab/besides_return -> slab/dma_rd_handler
decl_channel(DMA_ReadReq_Compressed, 1, slab_bin_dma_rd_req[SLAB_BIN_COUNT]);
// slab/besides_return_req_merger -> slab/besides_return
decl_channel(SlabRequest, 128, slab_besides_return_req);
// pcie/rx -> slab/besides_return_req_merger
decl_channel(SlabRequest, 128, host_slab_besides_return_req);
// hashtable/put/comparator_pipeline_two -> slab/besides_return_req_merger
decl_channel(SlabRequest, 128, hashtable_put_offline_slab_req);
// hashtable/put/comparator_pipeline_two -> hashtable/put/dma_wr_req_merger
decl_channel(SlabRequest, 128, hashtable_put_newline_slab_req);
// slab/besides_return -> slab/besides_return_req_merger
decl_channel(ulong, 128, slab_besides_return_res);
// slab/besides_return_req_merger-> hashtable/put/newline_handler
decl_channel(ulong, 128, slab_besides_return_res_newline);
// slab/besides_return_req_merger -> hashtable/put/offline_handler
decl_channel(ulong, 128, slab_besides_return_res_offline);
// src/hashtable/del/slab_return_req_merger -> src/slab/return
decl_channel(SlabReturn, 128, slab_return_req);
// src/hashtable/put/comparator_pipeline_one -> slab/return_req_merger
decl_channel(SlabReturn, 128, hashtable_put_slab_return_req_offline);
// src/hashtable/del/comparator -> slab/return_req_merger
decl_channel(SlabReturn, 128, hashtable_del_slab_return_req_offline);
// src/hashtable/del/comparator -> src/hashtable/del/slab_return_req_merger 
decl_channel(SlabReturn, 128, hashtable_del_hashtable_del_slab_return_req_line);
// slab/besides_return_req_merger -> slab/besides_return_req_merger
decl_channel(uchar, 128, slab_besides_return_req_context);
// slab/besides_return -> slab/return
decl_channel(ulong, 1, init_slab_return);
// slab/return -> slab/besides_return
decl_channel(bool, 128, add_8_slab_return_table_size[SLAB_BIN_COUNT]);
// host -> fpga
decl_channel(ulong8, 128, pcie_in);
// fpga -> host
decl_channel(ulong8, 128, pcie_out);
// slab/besides_return -> pcie/tx
decl_channel(ulong8, 128, slab_besides_return_sync);
// slab/besides_return -> hashtable/get/line_fetcher
decl_channel(ulong2, 1, init_hashtable_get_line_fetcher);
// slab/besides_return -> hashtable/del/line_fetcher
decl_channel(ulong2, 1, init_hashtable_del_line_fetcher);
// slab/besides_return -> hashtable/put/offline_handler 
decl_channel(ulong2, 1, init_hashtable_put_offline_handler);
// hashtable/get/comparator -> hashtable/get/line_fetcher
decl_channel(GetReq, 256, return_get_req);
// hashtable/get/line_fetcher -> hashtable/get/comparator
decl_channel(GetReq, 256, fetching_get_req);
// hashtable/line_fetcher/line_fetcher_rd_handler -> hashtable/line_fetcher/line_fetcher_rd_handler
decl_channel(DmaContext, 128, line_fetcher_dma_rd_handler_context);
// hashtable/get/line_fetcher -> hashtable/line_fetcher/line_fetcher_rd_handler
decl_channel(DMA_ReadReq_Compressed, 128, line_fetcher_get_dma_rd_req);
// hashtable/line_fetcher/line_fetcher_rd_handler -> hashtable/get/comparator
decl_channel(ulong8, 256, line_fetcher_get_dma_rd_res);
// hashtable/get/comparator -> hashtable/get/res_merger
decl_channel(GetRes, 256, get_inline_res);
// hashtable/get/offline_value_handler -> hashtable/get/res_merger
decl_channel(GetRes, 256, get_offline_res);
// hashtable/get/comparator -> dma/rd_manager
decl_channel(DMA_ReadReq_Compressed, 128, slab_fetcher_get_offline_dma_rd_req);
// hashtable/get/comparator -> hashtable/get/offline_value_handler
decl_channel(GetOfflineType, 128, slab_fetcher_get_offline_dma_rd_res_size_with_net_meta);
// dma/rd_manager -> hashtable/get/offline_value_handler
decl_channel(ulong4, 256, slab_fetcher_get_offline_dma_rd_res);
// slab/besides_return -> hashtable/get/comparator
decl_channel(ulong, 1, init_hashtable_get_comparator);
// slab/besides_return -> hashtable/del/comparator
decl_channel(ulong2, 1, init_hashtable_del_comparator);
// hashtable/del/comparator -> hashtable/del/line_fetcher
decl_channel(DelReq, 256, return_del_req);
// hashtable/del/line_fetcher -> hashtable/del/comparator
decl_channel(DelReq, 256, fetching_del_req);
// hashtable/del/line_fetcher -> hashtable/line_fetcher/line_fetcher_rd_handler
decl_channel(DMA_ReadReq_Compressed, 128, line_fetcher_del_dma_rd_req);
// hashtable/line_fetcher/line_fetcher_rd_handler -> hashtable/del/comparator 
decl_channel(ulong8, 256, line_fetcher_del_dma_rd_res);
// hashtable/del/dma_wr_req_merger -> hashtable/dma_wr_req_merger/dma_wr_req_merger
decl_channel(DMA_WriteReq_Compressed, 256, hashtable_del_dma_wr_req);
// hashtable/del/comparator -> hashtable/del/dma_wr_req_merger
decl_channel(DMA_WriteReq_Compressed, 256, hashtable_del_dma_wr_req_0);
// hashtable/del/comparator -> hashtable/del/dma_wr_req_merger
decl_channel(DMA_WriteReq_Compressed, 256, hashtable_del_dma_wr_req_1);
// hashtable/put/comparator_pipeline_two -> hashtable/put/line_fetcher
decl_channel(PutReq, 256, return_put_req);
// hashtable/put/newline_handler -> hashtable/put/line_fetcher
decl_channel(PutReq, 256, newline_put_req);
// hashtable/put/line_fetcher -> hashtable/put/comparator_pipeline_one
decl_channel(PutReq, 256, fetching_put_req);
// hashtable/put/comparator_pipeline_two -> hashtable/put/offline_handler
decl_channel(PutOfflineType, 256, put_offline_handler);
// hashtable/put/line_fetcher -> hashtable/line_fetcher/line_fetcher_rd_handler
decl_channel(DMA_ReadReq_Compressed, 128, line_fetcher_put_dma_rd_req);
// hashtable/line_fetcher/line_fetcher_rd_handler -> hashtable/put/comparator_pipeline_one
decl_channel(ulong8, 256, line_fetcher_put_dma_rd_res);
// slab/besides_return -> hashtable/put/line_fetcher
decl_channel(ulong2, 1, init_hashtable_put_line_fetcher);
// slab/besides_return -> hashtable/put/comparator_pipeline_one
decl_channel(ulong2, 1, init_hashtable_put_comparator_pipeline_one);
// slab/besides_return -> hashtable/put/comparator_pipeline_two
decl_channel(ulong2, 1, init_hashtable_put_comparator_pipeline_two);
// hashtable/put/newline_handler -> hashtable/put/dma_wr_req_merger
decl_channel(DMA_WriteReq_Compressed, 256, hashtable_put_newline_dma_wr_req);
// hashtable/put/comparator_pipeline_two -> hashtable/put/dma_wr_req_merger
decl_channel(DMA_WriteReq_Compressed_Double, 128, hashtable_put_inline_dma_wr_req_double);
// hashtable/put/offline_handler -> hashtable/put/dma_wr_req_merger
decl_channel(DMA_WriteReq_Compressed_Double, 128, hashtable_put_offline_update_line_dma_wr_req_double);
// hashtable/put/offline_handler -> hashtable/put/dma_wr_req_merger
decl_channel(DMA_WriteReq_Compressed_Double, 128, hashtable_put_offline_update_slab_dma_wr_req_double);
// hashtable/put/comparator_two -> hashtable/put/res_merger
decl_channel(PutRes, 256, put_inline_res);
// hashtable/put/offline_handler -> hashtable/put/res_merger
decl_channel(PutRes, 256, put_offline_res);
// hashtable/put/dma_wr_req_merger -> hashtable/dma_wr_req_merger/dma_wr_req_merger
decl_channel(DMA_WriteReq_Compressed, 256, hashtable_put_dma_wr_req);
// slab/besides_return -> hashtable/put/newline_handler
decl_channel(ulong2, 1, init_hashtable_put_newline_handler);
// UNKNOWN -> hashtable/get/line_fetcher
decl_channel(GetReq, 256, input_get_req);
// UNKNOWN -> hashtable/del/line_fetcher
decl_channel(DelReq, 256, input_del_req);
// UNKNOWN -> hashtable/put/line_fetcher
decl_channel(PutReq, 256, input_put_req);
// UNKNOWN -> hashtable/add/line_fetcher
decl_channel(AddReq, 256, input_add_req);
// hashtable/get/res_merger -> UNKNOWN
decl_channel(GetRes, 256, output_get_res);
// hashtable/del/comparator -> UNKNOWN
decl_channel(DelRes, 256, output_del_res);
// hashtable/put/res_merger -> UNKNOWN
decl_channel(PutRes, 256, output_put_res);
// hashtable/add/res_merger -> UNKNOWN
decl_channel(AddRes, 256, output_add_res);
// hashtable/get/array_req_generator -> hashtable/get/line_fetcher
decl_channel(GetReq, 256, array_get_req);
// hashtable/get/res_merger -> hashtable/get/array_req_generator
decl_channel(ArrayGetReqInfo, 256, array_get_req_info);
// slab/besides_return -> hashtable/add/line_fetcher
decl_channel(ulong2, 1, init_hashtable_add_line_fetcher);
// slab/besides_return -> hashtable/add/comparator
decl_channel(ulong2, 1, init_hashtable_add_comparator);
// hashtable/add/array_req_generator -> hashtable/add/line_fetcher
decl_channel(AddReq, 256, array_add_req);
// hashtable/add/offline_handler -> hashtable/add/array_req_generator
decl_channel(ArrayAddReqInfo, 256, array_add_req_info);
// hashtable/add/comparator -> hashtable/add/line_fetcher
decl_channel(AddReq, 256, return_add_req);
// hashtable/add/line_fetcher -> hashtable/add/comparator
decl_channel(AddReq, 256, fetching_add_req);
// hashtable/add/line_fetcher -> hashtable/line_fetcher/line_fetcher_rd_handler
decl_channel(DMA_ReadReq_Compressed, 128, line_fetcher_add_dma_rd_req);
// hashtable/line_fetcher/line_fetcher_rd_handler -> hashtable/add/comparator
decl_channel(ulong8, 256, line_fetcher_add_dma_rd_res);
// hashtable/add/comparator -> hashtable/add/offline_handler
decl_channel(AddOfflineType, 128, slab_fetcher_add_offline_dma_rd_res_size_with_net_meta);
// hashtable/add/offline_handler -> hashtable/add/adder
decl_channel(AddOfflineParsed, 256, add_offline_parsed);
// hashtable/add/comparator -> hashtable/add/res_merger
decl_channel(AddRes, 256, add_inline_res);
// hashtable/add/adder -> hashtable/add/res_merger
decl_channel(AddRes, 256, add_offline_res);
// hashtable/add/comparator -> hashtable/add/dma_wr_req_merger
decl_channel(DMA_WriteReq_Compressed_Double, 256, hashtable_add_inline_update_line_dma_wr_req_double);
// hashtable/add/adder -> hashtable/add/dma_wr_req_merger
decl_channel(DMA_WriteReq_Compressed, 256, hashtable_add_offline_update_slab_dma_wr_req);
// hashtable/add/dma_wr_req_merger -> hashtable/dma_wr_req_merger/dma_wr_req_merger
decl_channel(DMA_WriteReq_Compressed, 256, hashtable_add_dma_wr_req);
// hashtable/add/comparator -> dma/rd_manager
decl_channel(DMA_ReadReq_Compressed, 128, slab_fetcher_add_offline_dma_rd_req);
// dma/rd_manager -> hashtable/add/offline_handler
decl_channel(ulong4, 256, slab_fetcher_add_offline_dma_rd_res);
// hashtable/put/comparator_pipeline_one -> hashtable/put/comparator_pipeline_two
decl_channel(PutComparatorIntermInfo, 256, put_comparator_interm_info);
