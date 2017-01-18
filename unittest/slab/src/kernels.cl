#ifdef __unix__
#define _CSIM
#define _UNIT_TEST_SLAB
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
#include "../inc/constant.hpp"
#include "../inc/datatype.hpp"
#include "../inc/unroll.hpp"
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

#ifdef _CSIM

#include <cstring>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sys/time.h>
#include <string>

using namespace std;

void host_dma_simulator() {
  int cnt = 0;
  
  while (1) {
    bool read_dma_rd_req, read_dma_wr_req;
    DMA_ReadReq rd_req;
    DMA_WriteReq wr_req;
    // read simulator
    rd_req.raw = read_channel_nb_altera(dma_rd_req, &read_dma_rd_req);
    if (read_dma_rd_req) {
      assert(rd_req.req.size % 32 == 0);
      ulong *mem_in_ulong = (ulong *)rd_req.req.address;
      ulong4 data;
      data.x = data.y = data.z = data.w = 0;
      int cnt = 0;
      while (rd_req.req.size) {
	switch (cnt) {
	case 0:
	  data.x = *mem_in_ulong;
	  break;
	case 1:
	  data.y = *mem_in_ulong;
	  break;
	case 2:
	  data.z = *mem_in_ulong;
	  break;
	case 3:
	  data.w = *mem_in_ulong;
	  break;
	default:
	  break;	 
	}
	mem_in_ulong ++;
	cnt = (cnt + 1) % 4;
	rd_req.req.size -= 8;
	if (cnt == 0) {
	  DMA_ReadRes rd_res;
	  rd_res.res.data = data;
	  write_channel_altera(dma_rd_res, rd_res.raw);
	  data.x = data.y = data.z = data.w = 0;
	}
      }
      if (cnt != 0) {
	DMA_ReadRes rd_res;
	rd_res.res.data = data;
	write_channel_altera(dma_rd_res, rd_res.raw);
      }
    }
	
    // write simulator
    wr_req.raw = read_channel_nb_altera(dma_wr_req, &read_dma_wr_req);
    if (read_dma_wr_req) {
      assert(wr_req.req.size % 8 == 0);
      uint size = wr_req.req.size;
      ulong *mem_in_ulong = (ulong *)wr_req.req.address;
      int cnt = 0;
      while (size) {
	while (size && cnt <= 3) {
	  switch (cnt) {
	  case 0:
	    *mem_in_ulong = wr_req.req.data.x;
	    break;
	  case 1:
	    *mem_in_ulong = wr_req.req.data.y;
	    break;
	  case 2:
	    *mem_in_ulong = wr_req.req.data.z;
	    break;
	  case 3:
	    *mem_in_ulong = wr_req.req.data.w;
	    break;
	  default:
	    break;
	  }
	  cnt ++;
	  mem_in_ulong ++;
	  size -= 32;
	}
	if (size) {
	  wr_req.raw = read_channel_altera(dma_wr_req);
	  cnt = 0 ;
	}
      }
    }
  }
}

ulong host_slab_available_table_base_addr[SLAB_BIN_COUNT];
ulong host_slab_return_table_base_addr[SLAB_BIN_COUNT];
uint* host_slab_available_table;
uint host_slab_available_tail_ptr[SLAB_BIN_COUNT];

// BIN size: 32B, 64B, 128B, 256B, 512B
void
host_init() {  
  ulong slab_start_addr, slab_end_addr;
    
  // allocate space for slab table
  host_slab_available_table = (uint *)malloc(1024ULL * 1024 * 1024 * 64);
  if (!host_slab_available_table) {
    cout << "mem allocate failed!" << endl;
    exit(0);
  }
  memset(host_slab_available_table, 0, sizeof(host_slab_available_table));

  // address initialization
  host_slab_available_table_base_addr[0] = (ulong)host_slab_available_table;
  host_slab_available_table_base_addr[1] = host_slab_available_table_base_addr[0] + (1ULL << (30 + 2));
  host_slab_available_table_base_addr[2] = host_slab_available_table_base_addr[1] + (1ULL << (29 + 2));
  host_slab_available_table_base_addr[3] = host_slab_available_table_base_addr[2] + (1ULL << (28 + 2));    
  host_slab_available_table_base_addr[4] = host_slab_available_table_base_addr[3] + (1ULL << (27 + 2));

  host_slab_return_table_base_addr[0] = host_slab_available_table_base_addr[4] + (1 << (26 + 2));
  host_slab_return_table_base_addr[1] = host_slab_return_table_base_addr[0] + (1 << (20 + 2));
  host_slab_return_table_base_addr[2] = host_slab_return_table_base_addr[1] + (1 << (20 + 2));
  host_slab_return_table_base_addr[3] = host_slab_return_table_base_addr[2] + (1 << (20 + 2));
  host_slab_return_table_base_addr[4] = host_slab_return_table_base_addr[3] + (1 << (20 + 2));

  slab_start_addr = host_slab_return_table_base_addr[4] + (1 << (20 + 2));
  slab_end_addr = slab_start_addr + (1ULL << 35); // 32GB for slab

  // write init signal to host_init_slab_dma & host_init_slab
  PCIE_Message pcie_message;
  ClSignal signal;
  for (int i = 0; i < SLAB_BIN_COUNT; i++) {
    signal.Sig.LParam[i] = host_slab_available_table_base_addr[i];
  }
  signal.Sig.Cmd = SIGNAL_INIT_SLAB;
  pcie_message.data.data = signal.raw.lo;
  write_channel_altera(pcie_in, pcie_message.raw);
  pcie_message.data.data = signal.raw.hi;
  write_channel_altera(pcie_in, pcie_message.raw);  

  ulong current_addr = slab_start_addr;
  // prepare for the initial slab contents
  for (int i = 0; i < SLAB_BIN_COUNT - 1; i ++) {
    // each bin has 102400 initial items
    int initial_num = 1024000;
    for (int j = 0; j < initial_num; j ++) {
      host_slab_available_table[((host_slab_available_table_base_addr[i] - host_slab_available_table_base_addr[0]) >> 2) + j]
	= (((current_addr - slab_start_addr) >> 5) << 2) | ((i == 0) ? 3 : (SLAB_BIN_COUNT - 1 - i));
      current_addr += (i == 0) ? 256 : 512;
    }
    host_slab_available_tail_ptr[i] = initial_num;
  }

  // allocate the lefting space to 512B-bin
  while (current_addr != slab_end_addr) {
    host_slab_available_table[
			      ((host_slab_available_table_base_addr[SLAB_BIN_COUNT - 1] - host_slab_available_table_base_addr[0]) >> 2)
			      + host_slab_available_tail_ptr[SLAB_BIN_COUNT - 1]
			      ]
      = (((current_addr - slab_start_addr) >> 5) << 2) | 0 ;
    current_addr += 512;
    host_slab_available_tail_ptr[SLAB_BIN_COUNT - 1] ++;
  }
}

ulong host_slab_available_head_ptr[SLAB_BIN_COUNT];
ulong host_slab_return_tail_ptr[SLAB_BIN_COUNT];

long getCurrentTimeInMs() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000000 + tv.tv_usec;
}

ulong host_slab_return_head_ptr[SLAB_BIN_COUNT];

void host_routine() {
  long lastTime = getCurrentTimeInMs();
  uint *host_slab_return_table = (uint *)host_slab_return_table_base_addr[0];

  ulong host_slab_return_table_range[SLAB_BIN_COUNT];
  ulong host_slab_available_table_range[SLAB_BIN_COUNT];
  ulong host_slab_return_table_tail_ptr_offset_mask[SLAB_BIN_COUNT];

  ulong host_slab_available_table_head_ptr_offset_mask[SLAB_BIN_COUNT];
  
  for (int i = 0; i < SLAB_BIN_COUNT; i ++) {
    host_slab_return_table_range[i] = 1 << 20;
    host_slab_available_table_range[i] = 1 << (30 - i);
    host_slab_return_table_tail_ptr_offset_mask[i] = (1 << 20) - 1;
    host_slab_return_head_ptr[i] = 0;
    host_slab_available_table_head_ptr_offset_mask[i] = (1 << (30 - i)) - 1;
  }
  
  while (1) {
    // run runtime at the interval of 1ms
    if (getCurrentTimeInMs() - lastTime > 1000)  {
      lastTime = getCurrentTimeInMs();
      // update host_slab_available_tail_ptr & host_slab_return_tail_ptr
      ClSignal signal;
      PCIE_Message pcie_message;

      signal.Sig.Cmd = SIGNAL_QUERY_AVAILABLE_HEAD_PTR;
      pcie_message.data.data = signal.raw.lo;
      write_channel_altera(pcie_in, pcie_message.raw);
      pcie_message.data.data = signal.raw.hi;
      write_channel_altera(pcie_in, pcie_message.raw);

      pcie_message.raw = read_channel_altera(pcie_out);
      signal.raw.lo = pcie_message.data.data;
      pcie_message.raw = read_channel_altera(pcie_out);
      signal.raw.hi = pcie_message.data.data;
      
      for (int i = 0; i < SLAB_BIN_COUNT; i ++) {
	host_slab_available_head_ptr[i] = signal.Sig.LParam[i];
      }

      // replenish available table if it's not enough
      for (int i = 0; i < SLAB_BIN_COUNT; i ++) {
	uint num_left = ((host_slab_available_table_range[i] | host_slab_available_tail_ptr[i]) - host_slab_available_head_ptr[i]) & host_slab_available_table_head_ptr_offset_mask[i];
	int idx = (i == 0) ? 3 : (SLAB_BIN_COUNT - 1);
	ulong addr_offset = (host_slab_available_table_base_addr[i] - (ulong)host_slab_available_table) >> 2;
	ulong host_available_table_replenish_start_threshold = 1024000 / 3;
	ulong host_available_table_replenish_stop_threshold = 1024000;	
	if (num_left < host_available_table_replenish_start_threshold) {
	  if (i == SLAB_BIN_COUNT - 1) {
	    cout << num_left << " " << host_slab_available_head_ptr[i] << " " << host_slab_available_tail_ptr[i] << endl;
	    puts("No enough space to replenish insuffcient slab bin, exit!");
	    puts("ERROR 1");
	    exit(0);
	  }
	  while (num_left < host_available_table_replenish_stop_threshold && idx > i) {
	    uint be_splitted_bin_num_left = ((host_slab_available_table_range[idx] | host_slab_available_tail_ptr[idx]) - host_slab_available_head_ptr[idx]) & host_slab_available_table_head_ptr_offset_mask[idx];
	    if (be_splitted_bin_num_left > host_available_table_replenish_start_threshold) {
	      host_slab_available_tail_ptr[idx] --;
	      host_slab_available_tail_ptr[idx] &= host_slab_available_table_head_ptr_offset_mask[idx];
	      uint item = host_slab_available_table[((host_slab_available_table_base_addr[idx] - (ulong)host_slab_available_table) >> 2) + (host_slab_available_tail_ptr[idx])];
	      uint addr = (item >> 2) << 2;
	      uint size = item & 3;
	      size += idx - i;
	      host_slab_available_table[addr_offset + (host_slab_available_tail_ptr[i] ++)] = addr | size;
	      host_slab_available_tail_ptr[i] &= host_slab_available_table_head_ptr_offset_mask[i];
	      num_left ++;
	    }
	    else {
	      idx --;
	      if (idx <= i) {
		puts("No enough space to replenish insuffcient slab bin, exit!");
		puts("ERROR 2");
		exit(0);
	      }
	    }
	  }
	}
      } // end: auto replenish
      
      signal.Sig.Cmd = SIGNAL_QUERY_RETURN_TAIL_PTR;
      pcie_message.data.data = signal.raw.lo;
      write_channel_altera(pcie_in, pcie_message.raw);
      pcie_message.data.data = signal.raw.hi;
      write_channel_altera(pcie_in, pcie_message.raw);

      pcie_message.raw = read_channel_altera(pcie_out);
      signal.raw.lo = pcie_message.data.data;
      pcie_message.raw = read_channel_altera(pcie_out);
      signal.raw.hi = pcie_message.data.data;
       
      for (int i = 0; i < SLAB_BIN_COUNT; i ++) {
	host_slab_return_tail_ptr[i] = signal.Sig.LParam[i];
      }

      // move the return back items into available table
      for (int i = 0; i < SLAB_BIN_COUNT; i ++) {
	ulong end_idx = (host_slab_return_head_ptr[i] <= host_slab_return_tail_ptr[i]) ?
	  host_slab_return_tail_ptr[i] : host_slab_return_tail_ptr[i] + host_slab_return_table_range[i];
	for (int j = host_slab_return_head_ptr[i]; j < end_idx; j ++) {
	  ulong _j = j & host_slab_return_table_tail_ptr_offset_mask[i];
	  host_slab_available_table[((host_slab_available_table_base_addr[i] - (ulong)host_slab_available_table) >> 2) + (host_slab_available_tail_ptr[i] ++)] =
	    host_slab_return_table[((host_slab_return_table_base_addr[i] - host_slab_return_table_base_addr[0]) >> 2) + _j];
 	  host_slab_available_tail_ptr[i] &= host_slab_available_table_head_ptr_offset_mask[i];
	}
	host_slab_return_head_ptr[i] = host_slab_return_tail_ptr[i];
      }
    } // end: 1ms timing
  }
}

bool sync_bool = false;
vector<ulong> addrVec;

const int slab_size = 31;
const int iter_cnt = 256;

void requester() {
  
  while (1) {
    for (int i = 0; i < iter_cnt; i ++) {
      ClSignal signal;
      signal.Sig.Cmd = SIGNAL_REQUEST;
      signal.Sig.LParam[0] = slab_size;
      write_channel_altera(slab_besides_return_req, signal.raw);
    }

    while (!sync_bool) {}
    for (int i = 0; i < iter_cnt; i ++) {
      ClSignal signal;
      signal.Sig.LParam[0] = slab_size;
      signal.Sig.LParam[1] = addrVec[i];
      write_channel_altera(slab_return_req, signal.raw);
    }
    addrVec.clear();
    sync_bool = false;
  }
}

void receiver() {
  ulong slab_start_addr = host_slab_return_table_base_addr[4] + (1 << (20 + 2));

  while (1) {
    while (sync_bool) {}
    for (int i = 0; i < iter_cnt; i ++) {
      ulong addr = read_channel_altera(slab_besides_return_res);
      cout << std::hex << addr - slab_start_addr << endl;
      assert(addr);
      addrVec.push_back(addr);
    }
    sync_bool = true;
  }
}

int main() {
  ios::sync_with_stdio(false);
  
  host_init();
  boost::thread t_host_routine(&host_routine);
  boost::thread t_host_dma_simulator(&host_dma_simulator);

  boost::thread t_dma_rd_manager(&dma_rd_manager);
  boost::thread t_dma_wr_manager(&dma_wr_manager);
  
  boost::thread t_pcie_tx(&pcie_tx);
  boost::thread t_pcie_rx(&pcie_rx);

  boost::thread t_slab_besides_return_req_merger(&slab_besides_return_req_merger);
  boost::thread t_slab_dma_rd_req_merger(&slab_dma_rd_req_merger);
  boost::thread t_slab_dma_rd_handler(&slab_dma_rd_handler);
  boost::thread t_slab_dma_wr_handler(&slab_dma_wr_handler);
  boost::thread t_slab_besides_return(&slab_besides_return);
  boost::thread t_slab_return(&slab_return);

  boost::thread t_requester(&requester);
  boost::thread t_receiver(&receiver);
  
  t_host_dma_simulator.join();
  
  return 0;
}

#endif

