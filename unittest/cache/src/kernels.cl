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
#include "dma/wr_mux.cpp"
#include "dma/rd_mux.cpp"
#include "dma/rd_demux.cpp"
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

uint* host_slab_available_table;

typedef struct __attribute__((packed)) {
  ulong8 data;
  ulong flag;
}DramDataType;

DramDataType *dram;

void host_init() {
  dram = (DramDataType *)malloc(1ULL << 32);
  host_slab_available_table = (uint *)malloc(1024ULL * 1024 * 1024 * 64);
  memset(dram, 0, sizeof(dram));
  memset(host_slab_available_table, 0, sizeof(host_slab_available_table));
}

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
      wr_req.req.address += start_addr;
      cout << "sim0, wr addr: " << hex << wr_req.req.address << endl;
      
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
      rd_req.req.address += start_addr;
      cout << "sim0, rd addr: " << rd_req.req.address << endl;
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
      wr_req.req.address += start_addr;
      cout << "sim1, wr addr: " << hex << wr_req.req.address << endl;
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
      rd_req.req.address += start_addr;
      cout << "sim1, rd addr: " << hex << rd_req.req.address << endl;
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

ulong8 random_ulong8() {
  ulong8 data;
  data.lo.x = random();
  data.lo.y = random();
  data.lo.z = random();
  data.lo.w = random();
  data.hi.x = random();
  data.hi.y = random();
  data.hi.z = random();
  data.hi.w = random();
  return data;
}

ulong4 random_ulong4() {
  ulong4 data;
  data.x = random();
  data.y = random();
  data.z = random();
  data.w = random();
  return data;
}

bool operator ==(ulong4 a, ulong4 b) {
  return a.x == b.x &&
    a.y == b.y &&
    a.z == b.z &&
    a.w == b.w;
}

void test() {
  DMA_WriteReq dma_wrreq;
  DMA_ReadReq dma_rdreq;
  ulong4 dma_rdres;
  
  dma_wrreq.req.address = 0x100;
  dma_wrreq.req.size = 64;
  ulong8 data0 = random_ulong8();
  dma_wrreq.req.data = data0.lo;
  write_channel_nb_altera(dma_wr_req, dma_wrreq.raw);
  dma_wrreq.req.data = data0.hi;
  write_channel_nb_altera(dma_wr_req, dma_wrreq.raw);

  usleep(10000);
  
  dma_rdreq.req.address = 0x100;
  dma_rdreq.req.size = 64;
  write_channel_nb_altera(dma_rd_req, dma_rdreq.raw);
  
  dma_rdres = read_channel_altera(dma_rd_res);
  assert(dma_rdres == data0.lo);
  dma_rdres = read_channel_altera(dma_rd_res);
  assert(dma_rdres == data0.hi);

  usleep(10000);
    
  dma_rdreq.req.address = 0x100;
  dma_rdreq.req.size = 64;
  write_channel_nb_altera(dma_rd_req, dma_rdreq.raw);
  
  dma_rdres = read_channel_altera(dma_rd_res);
  assert(dma_rdres == data0.lo);
  dma_rdres = read_channel_altera(dma_rd_res);
  assert(dma_rdres == data0.hi);
    
  cout << "fin!" << endl;
}

int main() {
  srand(time(NULL));
  host_init();
  boost::thread t_dram_simulator(&dram_simulator);
  boost::thread t_host_dma_simulator0(&host_dma_simulator0);
  boost::thread t_host_dma_simulator1(&host_dma_simulator1);  
  boost::thread t_test(&test);
  
  boost::thread t_dma_rd_mux(&dma_rd_mux);
  boost::thread t_dma_wr_mux(&dma_wr_mux);
  boost::thread t_dma_rd_demux(&dma_rd_demux);
  
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
    
  t_dram_simulator.join();
    
  return 0;
}
#endif
