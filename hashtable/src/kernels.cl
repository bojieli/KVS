#ifdef __unix__
#define _CSIM
#endif

#ifdef _CSIM
#define _CL_VOID void
#include <cassert>
#include "sim/channel.cpp"
#include "../inc/cl_platform.h"
#define decl_channel(TYPE, DEPTH, NAME) channel<TYPE, DEPTH> NAME
#else
#define _CL_VOID \
  __attribute__((autorun))\
  __attribute__((max_global_work_dim(0)))\
  kernel void
#define decl_channel(TYPE, DEPTH, NAME) channel TYPE NAME __attribute__((depth(DEPTH)));
#define assert(STMT) 
#endif

/**********************************/
#include "../inc/clsignal.h"
#include "../inc/pciemessage.h"
#include "../inc/dmawritereq.h"
#include "../inc/dmareadreq.h"
#include "../inc/dmareadres.h"
#include "../inc/memreadreq.h"
#include "../inc/memreadres.h"
#include "../inc/memwritereq.h"
#include "../inc/cachefunc.h"
#include "../inc/constant.hpp"
#include "../inc/datatype.hpp"
#include "../inc/hashfunc.hpp"
#include "../inc/unroll.hpp"
/**********************************/

/**********************************/
#include "chan/channels.cpp"
/**********************************/

/**********************************/
#include "dma/rd_manager.cpp"
#include "dma/wr_manager.cpp"
#include "dma/wr_mux.cpp"
#include "dma/rd_mux.cpp"
#include "dma/rd_demux.cpp"
/**********************************/

/**********************************/
#include "pcie/tx.cpp"
#include "pcie/rx.cpp"
/**********************************/

/**********************************/
#include "slab/besides_return_req_merger.cpp"
#include "slab/dma_rd_req_merger.cpp"
#include "slab/dma_rd_handler.cpp"
#include "slab/dma_wr_handler.cpp"
#include "slab/besides_return.cpp"
#include "slab/return.cpp"
#include "slab/return_req_merger.cpp"
/**********************************/

/**********************************/
#include "hashtable/get/comparator.cpp"
#include "hashtable/get/line_fetcher.cpp"
#include "hashtable/get/res_merger.cpp"
#include "hashtable/get/offline_handler.cpp"
#include "hashtable/get/array_req_generator.cpp"
/**********************************/

/**********************************/
#include "hashtable/del/dma_wr_req_merger.cpp"
#include "hashtable/del/line_fetcher.cpp"
#include "hashtable/del/comparator.cpp"
/**********************************/

/**********************************/
#include "hashtable/put/res_merger.cpp"
#include "hashtable/put/dma_wr_req_merger.cpp"
#include "hashtable/put/offline_handler.cpp"
#include "hashtable/put/line_fetcher.cpp"
#include "hashtable/put/newline_handler.cpp"
#include "hashtable/put/comparator_pipeline_one.cpp"
#include "hashtable/put/comparator_pipeline_two.cpp"
/**********************************/

/**********************************/
#include "hashtable/add/comparator.cpp"
#include "hashtable/add/line_fetcher.cpp"
#include "hashtable/add/array_req_generator.cpp"
#include "hashtable/add/offline_handler.cpp"
#include "hashtable/add/adder.cpp"
#include "hashtable/add/res_merger.cpp"
#include "hashtable/add/dma_wr_req_merger.cpp"
/**********************************/

/**********************************/
#include "cache/cache_res_merger.cpp"
#include "cache/dram_line_comparator.cpp"
#include "cache/dram_writer.cpp"
#include "cache/dma_rd_filter.cpp"
#include "cache/dram_wr_req_splitter.cpp"
#include "cache/dma_res_merger.cpp"
#include "cache/dram_rd_mux.cpp"
#include "cache/redirect_pcie_wr_req_formatter.cpp"
#include "cache/dma_wr_filter.cpp"
#include "cache/dram_rd_req_splitter.cpp"
/**********************************/

/**********************************/
#include "hashtable/line_fetcher/line_fetcher_rd_handler.cpp"
/**********************************/

#ifdef _CSIM
int main() {
  return 0;
}
#endif
