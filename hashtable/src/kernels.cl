#define _CSIM

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
#include "../inc/constant.hpp"
#include "../inc/datatype.hpp"
/**********************************/

/**********************************/
#include "chan/channels.cpp"
/**********************************/

/**********************************/
#include "dma/rd_manager.cpp"
#include "dma/wr_manager.cpp"
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
/**********************************/

/**********************************/
#include "hashtable/get/comparator.cpp"
#include "hashtable/get/line_fetcher.cpp"
#include "hashtable/get/res_merger.cpp"
#include "hashtable/get/offline_value_handler.cpp"
/**********************************/

/**********************************/
#include "hashtable/del/dma_wr_req_merger.cpp"
#include "hashtable/del/line_fetcher.cpp"
#include "hashtable/del/comparator.cpp"
#include "hashtable/del/slab_return_req_merger.cpp"
/**********************************/

/**********************************/
#include "hashtable/put/res_merger.cpp"
#include "hashtable/put/dma_wr_req_merger.cpp"
#include "hashtable/put/offline_handler.cpp"
#include "hashtable/put/line_fetcher.cpp"
#include "hashtable/put/comparator.cpp"
#include "hashtable/put/newline_handler.cpp"
/**********************************/

/**********************************/
#include "hashtable/line_fetcher/line_fetcher_rd_handler.cpp"
/**********************************/

#ifdef _CSIM
int main() {
  return 0;
}
#endif