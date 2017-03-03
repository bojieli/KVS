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

typedef struct __attribute__((packed)) {
  ulong8 data;
  ulong flag;
}DramDataType;

DramDataType *dram;

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
#include "hashtable/line_fetcher/line_fetcher_rd_handler.cpp"
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

#ifdef _CSIM

#include <cstring>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sys/time.h>
#include <string>

using namespace std;

ulong host_slab_available_table_base_addr[SLAB_BIN_COUNT];
ulong host_slab_return_table_base_addr[SLAB_BIN_COUNT];
uint* host_slab_available_table;
uint host_slab_available_tail_ptr[SLAB_BIN_COUNT];

void dram_simulator() {
  while (1) {
    bool read_mem_rdreq;
    Mem_ReadReq mem_rdreq;
    mem_rdreq = read_channel_nb_altera(mem_rd_req, &read_mem_rdreq);
    if (read_mem_rdreq) {
      DramDataType dramData = dram[mem_rdreq.address];
      Mem_ReadRes mem_rdres;
      mem_rdres.data = dramData.data;
      mem_rdres.flag = dramData.flag;
      bool dummy = write_channel_nb_altera(mem_rd_res, mem_rdres);
      assert(dummy);
    }

    bool read_mem_wrreq;
    Mem_WriteReq mem_wrreq;
    mem_wrreq = read_channel_nb_altera(mem_wr_req, &read_mem_wrreq);
    if (read_mem_wrreq) {
      DramDataType dramData;
      dramData.data = mem_wrreq.data;
      dramData.flag = mem_wrreq.flag;
      dram[mem_wrreq.address] = dramData;
    }
  }
}

// BIN size: 32B, 64B, 128B, 256B, 512B
void
host_init() {
  dram = (DramDataType *)malloc(1ULL << 32);
  host_slab_available_table = (uint *)malloc(1024ULL * 1024 * 1024 * 64);
  memset(dram, 0, sizeof(dram));
  memset(host_slab_available_table, 0, sizeof(host_slab_available_table));
  
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

void host_dma_simulator0() {
  int cnt = 0;
  ulong start_addr = (ulong)host_slab_available_table;
  ulong end_addr = start_addr + 1024ULL * 1024 * 1024 * 64;
  
  while (1) {
    bool read_dma_rd_req, read_dma_wr_req;
    DMA_ReadReq rd_req;
    DMA_WriteReq wr_req;

    // write simulator
    wr_req.raw = read_channel_nb_altera(dma0_wr_req, &read_dma_wr_req);
    if (read_dma_wr_req) {
      assert(wr_req.req.size % 8 == 0);
      uint size = wr_req.req.size;
      assert(wr_req.req.address >= start_addr);
      assert(wr_req.req.address < end_addr);
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
	  size -= 8;
	}
	if (size) {
	  wr_req.raw = read_channel_altera(dma0_wr_req);
	  cnt = 0 ;
	}
      }
    }
    
    // read simulator
    rd_req.raw = read_channel_nb_altera(dma0_rd_req, &read_dma_rd_req);
    if (read_dma_rd_req) {
      assert(rd_req.req.size % 32 == 0);
      assert(rd_req.req.address >= start_addr);
      assert(rd_req.req.address < end_addr);
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
	  write_channel_altera(dma0_rd_res, rd_res.raw);
	  data.x = data.y = data.z = data.w = 0;
	}
      }
      if (cnt != 0) {
	DMA_ReadRes rd_res;
	rd_res.res.data = data;
	write_channel_altera(dma0_rd_res, rd_res.raw);
      }
    }	
  }
}

void host_dma_simulator1() {
  int cnt = 0;
  ulong start_addr = (ulong)host_slab_available_table;
  ulong end_addr = start_addr + 1024ULL * 1024 * 1024 * 64;
  
  while (1) {
    bool read_dma_rd_req, read_dma_wr_req;
    DMA_ReadReq rd_req;
    DMA_WriteReq wr_req;

    // write simulator
    wr_req.raw = read_channel_nb_altera(dma1_wr_req, &read_dma_wr_req);
    if (read_dma_wr_req) {
      assert(wr_req.req.size % 8 == 0);
      uint size = wr_req.req.size;
      assert(wr_req.req.address >= start_addr);
      assert(wr_req.req.address < end_addr);
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
	  size -= 8;
	}
	if (size) {
	  wr_req.raw = read_channel_altera(dma1_wr_req);
	  cnt = 0 ;
	}
      }
    }
    
    // read simulator
    rd_req.raw = read_channel_nb_altera(dma1_rd_req, &read_dma_rd_req);
    if (read_dma_rd_req) {
      assert(rd_req.req.size % 32 == 0);
      assert(rd_req.req.address >= start_addr);
      assert(rd_req.req.address < end_addr);
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
	  write_channel_altera(dma1_rd_res, rd_res.raw);
	  data.x = data.y = data.z = data.w = 0;
	}
      }
      if (cnt != 0) {
	DMA_ReadRes rd_res;
	rd_res.res.data = data;
	write_channel_altera(dma1_rd_res, rd_res.raw);
      }
    }	
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

inline void string2key(string s, uchar &key_size, ulong4 &key) {
  assert(s.size() <= 32);
  key_size = s.size();
  uchar key_slice[32];
  for (int i = 0; i < 32; i ++) {
    if (i < s.size()) {
      key_slice[i] = s[i];
    }
    else {
      key_slice[i] = 0;
    }
  }
  ulong tmp[4];
  for (int i = 0; i < 4; i ++) {
    tmp[i] =
      ((ulong)key_slice[0 + (i << 3)]) << 56 |
      ((ulong)key_slice[1 + (i << 3)]) << 48 |
      ((ulong)key_slice[2 + (i << 3)]) << 40 |
      ((ulong)key_slice[3 + (i << 3)]) << 32 |
      ((ulong)key_slice[4 + (i << 3)]) << 24 |
      ((ulong)key_slice[5 + (i << 3)]) << 16 |
      ((ulong)key_slice[6 + (i << 3)]) << 8 |
      ((ulong)key_slice[7 + (i << 3)]) << 0;
  }
  ulong4 ret;
  key.x = tmp[0];
  key.y = tmp[1];
  key.z = tmp[2];
  key.w = tmp[3];
}

bool operator ==(ulong4 v1, ulong4 v2) {
  return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w;
}

void test() {
  PutReq put_req;
  PutRes put_res;
  AddReq add_req;
  AddRes add_res;
  GetReq get_req;
  GetRes get_res;
  DelReq del_req;
  DelRes del_res;
  uint hash1bak;
  
  // inline put
  put_req.key_size = 6;
  put_req.key.x = 0x7582754283870000;
  put_req.key.y = 0;
  put_req.key.z = 0;
  put_req.key.w = 0;
  put_req.hash1 = hash_func1(&put_req.key);
  put_req.hash2 = hash_func2(&put_req.key);
  put_req.val_size = 4;
  put_req.val.x = 0x6838129400000000;
  put_req.val.y = 0;
  put_req.val.z = 0;
  put_req.val.w = 0;
  put_req.has_last = 0;
  write_channel_altera(input_put_req, put_req);
  usleep(200000);

  put_res = read_channel_altera(output_put_res);
  assert(put_res.found);
  assert(put_res.key_size == 6);
  assert(put_res.key.x == 0x7582754283870000);

  // inline get
  get_req.is_array_first = 0;
  get_req.key_size = 6;
  get_req.key.x = 0x7582754283870000;
  get_req.hash1 = hash_func1(&put_req.key);
  get_req.hash2 = hash_func2(&put_req.key);
  get_req.has_last = 0;
  write_channel_altera(input_get_req, get_req);
  
  get_res = read_channel_altera(output_get_res);
  assert(get_res.found);
  assert(get_res.key_size == 6);
  assert(get_res.key.x == 0x7582754283870000);
  assert(get_res.val_size == 4);
  assert(get_res.val.x == 0x6838129400000000);
  
  // inline add
  add_req.key_size = 6;
  add_req.key.x = 0x7582754283870000;
  add_req.key.y = 0;  
  add_req.key.z = 0;
  add_req.key.w = 0;
  add_req.hash1 = hash_func1(&add_req.key);
  add_req.hash2 = hash_func2(&add_req.key);
  add_req.delta = 1;
  add_req.has_last = 0;
  add_req.is_array = 0;
  add_req.is_array_first = 0;
  write_channel_altera(input_add_req, add_req);
  
  add_res = read_channel_altera(output_add_res);
  assert(add_res.found);
  assert(add_res.key_size == 6);
  assert(add_res.key.x == 0x7582754283870000);
  usleep(200000);
    
  // inline get
  get_req.is_array_first = 0;
  get_req.key_size = 6;
  get_req.key.x = 0x7582754283870000;
  get_req.hash1 = hash_func1(&put_req.key);
  get_req.hash2 = hash_func2(&put_req.key);
  get_req.has_last = 0;
  write_channel_altera(input_get_req, get_req);

  get_res = read_channel_altera(output_get_res);
  assert(get_res.found);
  assert(get_res.key_size == 6);
  assert(get_res.key.x == 0x7582754283870000);
  assert(get_res.val_size == 4);
  assert(get_res.val.x == 0x6838129500000000);
  usleep(200000);

  // offline put
  put_req.key_size = 14;
  put_req.key.x = 0x5858282843850938;
  put_req.key.y = 0x9483736561720000;
  put_req.key.z = 0;
  put_req.key.w = 0;
  put_req.hash1 = hash_func1(&put_req.key);
  put_req.hash2 = hash_func2(&put_req.key);
  put_req.val_size = 4;
  put_req.val.x = 0x7858278400000000;
  put_req.val.y = 0;
  put_req.val.z = 0;
  put_req.val.w = 0;
  put_req.has_last = 0;
  write_channel_altera(input_put_req, put_req);
  usleep(200000);
  
  put_res = read_channel_altera(output_put_res);
  assert(put_res.found);
  assert(put_res.key_size == 14);
  assert(put_res.key.x == 0x5858282843850938);
  assert(put_res.key.y == 0x9483736561720000);
  assert(put_res.key.z == 0);
  assert(put_res.key.w == 0);
  usleep(200000);

  // offline add
  add_req.key_size = 14;
  add_req.key.x = 0x5858282843850938;
  add_req.key.y = 0x9483736561720000; 
  add_req.key.z = 0;
  add_req.key.w = 0;
  add_req.hash1 = hash_func1(&add_req.key);
  add_req.hash2 = hash_func2(&add_req.key);
  add_req.delta = 2;
  add_req.has_last = 0;
  add_req.is_array = 0;
  add_req.is_array_first = 0;
  write_channel_altera(input_add_req, add_req);

  add_res = read_channel_altera(output_add_res);
  assert(add_res.found);      
  assert(add_res.key_size == 14);
  assert(add_res.key.x == 0x5858282843850938);
  assert(add_res.key.y == 0x9483736561720000);
  assert(add_res.key.z == 0);
  assert(add_res.key.w == 0);
  usleep(200000);

  // offline get
  get_req.is_array_first = 0;
  get_req.key_size = 14;
  get_req.key.x = 0x5858282843850938;
  get_req.key.y = 0x9483736561720000;
  get_req.key.z = 0;
  get_req.key.w = 0;
  get_req.hash1 = hash_func1(&get_req.key);
  get_req.hash2 = hash_func2(&get_req.key);
  get_req.has_last = 0;
  write_channel_altera(input_get_req, get_req);

  get_res = read_channel_altera(output_get_res);
  assert(get_res.found);
  assert(get_res.key_size == 14);
  assert(get_res.key.x == 0x5858282843850938);
  assert(get_res.key.y == 0x9483736561720000);
  assert(get_res.key.z == 0);
  assert(get_res.key.w == 0);
  assert(get_res.val_size == 4);
  assert(get_res.val.x == 0x7858278600000000);

  //**********CUT HERE************
  uchar req_key_size;
  ulong4 req_key;

  // array put
  string2key("$arr", put_req.key_size, put_req.key);  
  req_key_size = put_req.key_size;
  req_key = put_req.key; 
  put_req.hash1 = hash_func1(&put_req.key);
  put_req.hash2 = hash_func2(&put_req.key);
  
  put_req.val_size = 86;  
  put_req.val.x = 0x0002129417387585;
  put_req.val.y = 0x1949682545784392;
  put_req.val.z = 0x4997828158829187;
  put_req.val.w = 0x9384761258689146;
  put_req.has_last = 0;
  write_channel_altera(input_put_req, put_req);
  
  put_req.val.x = 0x9585682577991746;
  put_req.val.y = 0x2848568919298487;
  put_req.val.z = 0x4294869585869813;
  put_req.val.w = 0x1905968645861374;
  write_channel_altera(input_put_req, put_req);

  put_req.val.x = 0x2359685484210960;
  put_req.val.y = 0x5482828355819838;
  put_req.val.z = 0x8829486553320000;
  put_req.val.w = 0;
  write_channel_altera(input_put_req, put_req);
  usleep(200000);

  put_res = read_channel_altera(output_put_res);
  assert(put_res.found);
  assert(put_res.key_size == req_key_size);
  assert(put_res.key == req_key);

  string2key("$arr1", put_req.key_size, put_req.key);  
  req_key_size = put_req.key_size;
  req_key = put_req.key; 
  put_req.hash1 = hash_func1(&put_req.key);
  put_req.hash2 = hash_func2(&put_req.key);
  
  put_req.val_size = 32;  
  put_req.val.x = 0x3423423282184374;
  put_req.val.y = 0x9584716326546548;
  put_req.val.z = 0x1838485868678728;
  put_req.val.w = 0x7982945692434864;
  put_req.has_last = 0;
  write_channel_altera(input_put_req, put_req);
  usleep(200000);
  
  put_res = read_channel_altera(output_put_res);
  assert(put_res.found);
  assert(put_res.key_size == req_key_size);
  assert(put_res.key == req_key);  

  // array get 
  string2key("$arr", get_req.key_size, get_req.key);  
  req_key_size = get_req.key_size;
  req_key = get_req.key; 
  get_req.hash1 = hash_func1(&get_req.key);
  get_req.hash2 = hash_func2(&get_req.key);
  get_req.has_last = false;
  get_req.is_array_first = true;
  
  write_channel_altera(input_get_req, get_req);
  usleep(200000);

  get_res = read_channel_altera(output_get_res);
  assert(get_res.found);
  assert(get_res.key_size == req_key_size);
  assert(get_res.key == req_key);
  
  assert(get_res.val_size == 86);
  assert(get_res.val.x == 0x0002129417387585);
  assert(get_res.val.y == 0x1949682545784392);
  assert(get_res.val.z == 0x4997828158829187);
  assert(get_res.val.w == 0x9384761258689146);
  
  get_res = read_channel_altera(output_get_res);
  assert(get_res.val.x == 0x9585682577991746);
  assert(get_res.val.y == 0x2848568919298487);
  assert(get_res.val.z == 0x4294869585869813);
  assert(get_res.val.w == 0x1905968645861374); 

  get_res = read_channel_altera(output_get_res);
  assert(get_res.val.x == 0x2359685484210960);
  assert(get_res.val.y == 0x5482828355819838);
  assert(get_res.val.z == 0x8829486553320000);
  assert(get_res.val.w == 0);
  
  get_res = read_channel_altera(output_get_res);

  string2key("$arr1", req_key_size, req_key);
  assert(get_res.found);
  assert(get_res.key_size == req_key_size);
  assert(get_res.key == req_key);
  assert(get_res.val_size == 32);
  
  assert(get_res.val.x == 0x3423423282184374);
  assert(get_res.val.y == 0x9584716326546548);
  assert(get_res.val.z == 0x1838485868678728);
  assert(get_res.val.w == 0x7982945692434864);
  usleep(200000);

  // array add
  string2key("$arr", add_req.key_size, add_req.key);  
  req_key_size = add_req.key_size;
  req_key = add_req.key; 
  add_req.hash1 = hash_func1(&add_req.key);
  add_req.hash2 = hash_func2(&add_req.key);
  add_req.delta = 1;
  add_req.has_last = 0;
  add_req.is_array_first = 1;
  write_channel_altera(input_add_req, add_req);
  usleep(200000);
  
  add_res = read_channel_altera(output_add_res);
  assert(add_res.found);
  assert(add_res.key_size == req_key_size);
  assert(add_res.key == req_key);

  // array get 
  string2key("$arr", get_req.key_size, get_req.key);  
  req_key_size = get_req.key_size;
  req_key = get_req.key; 
  get_req.hash1 = hash_func1(&get_req.key);
  get_req.hash2 = hash_func2(&get_req.key);
  get_req.has_last = false;
  get_req.is_array_first = true;
  write_channel_altera(input_get_req, get_req);
  usleep(200000);

  get_res = read_channel_altera(output_get_res);
  assert(get_res.found);
  assert(get_res.key_size == req_key_size);
  assert(get_res.key == req_key);
  
  assert(get_res.val_size == 86);
  assert(get_res.val.x == 0x0002129417397585);
  assert(get_res.val.y == 0x194a682545794392);
  assert(get_res.val.z == 0x4998828158839187);
  assert(get_res.val.w == 0x9385761258699146);

  get_res = read_channel_altera(output_get_res);
  assert(get_res.val.x == 0x95866825779a1746);
  assert(get_res.val.y == 0x28495689192a8487);
  assert(get_res.val.z == 0x4295869585879813);
  assert(get_res.val.w == 0x1906968645871374);

  get_res = read_channel_altera(output_get_res);
  assert(get_res.val.x == 0x235a685484220960);
  assert(get_res.val.y == 0x5483828355829838);
  assert(get_res.val.z == 0x882a486553330000);
  assert(get_res.val.w == 0);

  get_res = read_channel_altera(output_get_res);
  string2key("$arr1", req_key_size, req_key);
  assert(get_res.key_size == req_key_size);
  assert(get_res.key == req_key);

  assert(get_res.val.x == 0x3423423382184375);
  assert(get_res.val.y == 0x9584716426546549);
  assert(get_res.val.z == 0x1838485968678729);
  assert(get_res.val.w == 0x7982945792434865);
  
  cout << "passed" << endl;
}

void set_affinity(boost::thread &thread, uint core_id) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);
  pthread_setaffinity_np(
			 thread.native_handle(),
			 sizeof(cpu_set_t),
			 &cpuset
			 );
}

int main() {
  ios::sync_with_stdio(false);

  host_init();
  boost::thread t_host_routine(&host_routine);
  boost::thread t_dram_simulator(&dram_simulator);    
  boost::thread t_host_dma_simulator0(&host_dma_simulator0);
  boost::thread t_host_dma_simulator1(&host_dma_simulator1);
  
  boost::thread t_dma_rd_manager(&dma_rd_manager);
  boost::thread t_dma_wr_manager(&dma_wr_manager);
  boost::thread t_dma_rd_mux(&dma_rd_mux);
  boost::thread t_dma_wr_mux(&dma_wr_mux);
  boost::thread t_dma_rd_demux(&dma_rd_demux);
  
  boost::thread t_pcie_tx(&pcie_tx);
  boost::thread t_pcie_rx(&pcie_rx);

  boost::thread t_slab_besides_return_req_merger(&slab_besides_return_req_merger);
  boost::thread t_slab_dma_rd_req_merger(&slab_dma_rd_req_merger);
  boost::thread t_slab_dma_rd_handler(&slab_dma_rd_handler);
  boost::thread t_slab_dma_wr_handler(&slab_dma_wr_handler);
  boost::thread t_slab_besides_return(&slab_besides_return); 
  boost::thread t_slab_return(&slab_return);
  boost::thread t_slab_return_req_merger(&slab_return_req_merger);

  boost::thread t_hashtable_get_comparator(&hashtable_get_comparator);
  boost::thread t_hashtable_get_line_fetcher(&hashtable_get_line_fetcher);
  boost::thread t_hashtable_get_offline_handler(&hashtable_get_offline_handler);
  boost::thread t_hashtable_get_array_req_generator(&hashtable_get_array_req_generator);
  boost::thread t_hashtable_get_res_merger(&hashtable_get_res_merger);

  boost::thread t_hashtable_del_comparator(&hashtable_del_comparator);
  boost::thread t_hashtable_del_dma_wr_req_merger(&hashtable_del_dma_wr_req_merger);  
  boost::thread t_hashtable_del_line_fetcher(&hashtable_del_line_fetcher);
  
  boost::thread t_hashtable_put_res_merger(&hashtable_put_res_merger);
  boost::thread t_hashtable_put_dma_wr_req_merger(&hashtable_put_dma_wr_req_merger);
  boost::thread t_hashtable_put_offline_handler(&hashtable_put_offline_handler);
  boost::thread t_hashtable_put_line_fetcher(&hashtable_put_line_fetcher);
  boost::thread t_hashtable_put_newline_handler(&hashtable_put_newline_handler);
  boost::thread t_hashtable_put_comparator_pipeline_one(&hashtable_put_comparator_pipeline_one);
  boost::thread t_hashtable_put_comparator_pipeline_two(&hashtable_put_comparator_pipeline_two);

  boost::thread t_hashtable_add_comparator(hashtable_add_comparator);
  boost::thread t_hashtable_add_line_fetcher(hashtable_add_line_fetcher);
  boost::thread t_hashtable_add_array_req_generator(hashtable_add_array_req_generator);
  boost::thread t_hashtable_add_offline_handler(hashtable_add_offline_handler);
  boost::thread t_hashtable_add_adder(hashtable_add_adder);
  boost::thread t_hashtable_add_res_merger(hashtable_add_res_merger);
  boost::thread t_hashtable_add_dma_wr_req_merger(hashtable_add_dma_wr_req_merger);

  boost::thread t_hashtable_line_fetcher_dma_rd_handler(&hashtable_line_fetcher_dma_rd_handler);

  boost::thread t_cache_cache_res_merger(&cache_cache_res_merger);
  boost::thread t_cache_dram_line_comparator(&cache_dram_line_comparator);
  boost::thread t_cache_dram_wr_req_splitter(&cache_dram_wr_req_splitter);
  boost::thread t_cache_dma_rd_filter(&cache_dma_rd_filter);
  boost::thread t_cache_dram_rd_mux(&cache_dram_rd_mux);
  boost::thread t_cache_redirect_pcie_wr_req_formatter(&cache_redirect_pcie_wr_req_formatter);
  boost::thread t_cache_dma_res_merger(&cache_dma_res_merger);
  boost::thread t_cache_dram_rd_req_splitter(&cache_dram_rd_req_splitter);
  boost::thread t_cache_dma_wr_filter(&cache_dma_wr_filter);
  boost::thread t_cache_dram_writer(&cache_dram_writer);
  
  boost::thread t_test(&test);
  
  t_host_dma_simulator0.join();

  return 0;
}

#endif
