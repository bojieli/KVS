_CL_VOID
hashtable_put_offline_handler() {
  bool is_valid_put_offline_handler = false;
  PutOfflineType show_ahead_put_offline_handler;
  PutOfflineType old_show_ahead_put_offline_handler;  
  ulong2 init = read_channel_altera(init_hashtable_put_offline_handler);
  ulong slab_start_addr = init.x;
  ulong line_start_addr = init.y;
  ulong slab_grant_addr;
  PutReq req;
  uchar val_in_byte[32];
  ushort val_size_left = 0;

  bool last_finished = true;
  bool first_write = false;
  
  while (1) {
    bool should_write_hashtable_put_offline_update_slab_dma_wr_req = false;
    bool should_write_hashtable_put_offline_update_line_dma_wr_req = false;
    Ulong16 val_hashtable_put_offline_update_line_dma_wr_req;
    val_hashtable_put_offline_update_line_dma_wr_req.valid2 = false;
    Ulong16 val_hashtable_put_offline_update_slab_dma_wr_req;
    val_hashtable_put_offline_update_slab_dma_wr_req.valid2 = false;
    bool should_write_put_offline_res = false;
    PutRes val_write_put_offline_res;
    
    if (!is_valid_put_offline_handler) {
      old_show_ahead_put_offline_handler = show_ahead_put_offline_handler;
      show_ahead_put_offline_handler = read_channel_nb_altera(put_offline_handler, &is_valid_put_offline_handler);
    }

    if (last_finished) {
      bool read_slab_besides_return_res;
      slab_grant_addr = read_channel_nb_altera(slab_besides_return_res_offline, &read_slab_besides_return_res);

      if (read_slab_besides_return_res) {
	last_finished = false;
	first_write = true;
	is_valid_put_offline_handler = false;
	uchar line_in_uchar[64];
#pragma unroll	
	for (int i = 0; i < 8; i ++) {
	  line_in_uchar[i] = ((ulong)((show_ahead_put_offline_handler.line.lo.x) << (i << 3))) >> 56;
	  line_in_uchar[i + 8] = ((ulong)((show_ahead_put_offline_handler.line.lo.y) << (i << 3))) >> 56;
	  line_in_uchar[i + 16] = ((ulong)((show_ahead_put_offline_handler.line.lo.z) << (i << 3))) >> 56;
	  line_in_uchar[i + 24] = ((ulong)((show_ahead_put_offline_handler.line.lo.w) << (i << 3))) >> 56;
	  line_in_uchar[i + 32] = ((ulong)((show_ahead_put_offline_handler.line.hi.x) << (i << 3))) >> 56;
	  line_in_uchar[i + 40] = ((ulong)((show_ahead_put_offline_handler.line.hi.y) << (i << 3))) >> 56;
	  line_in_uchar[i + 48] = ((ulong)((show_ahead_put_offline_handler.line.hi.z) << (i << 3))) >> 56;
	  line_in_uchar[i + 56] = ((ulong)((show_ahead_put_offline_handler.line.hi.w) << (i << 3))) >> 56;
	}	

	uchar put_idx = show_ahead_put_offline_handler.put_idx;
	req = show_ahead_put_offline_handler.req;
	val_size_left = req.val_size;

	uchar line_in_1bit[512];
#pragma unroll	      
	for (int i = 0; i < 512; i ++) {
	  line_in_1bit[i] = ((line_in_uchar[i >> 3]) >> (7 - (i & 7))) & 1;
	}

	uchar hash2addr_in_1bit[40];
#pragma unroll	
	for (int i = 0; i < 10; i ++) {
	  hash2addr_in_1bit[i] = (req.hash2 >> (9 - i)) & 1;
	}

	ulong addr = (slab_grant_addr - slab_start_addr) >> 5;
	
#pragma unroll
	for (int i = 10; i < 40; i ++) {
	  hash2addr_in_1bit[i] = (addr >> (29 - (i - 10))) & 1;
	}

	// modify data	
#pragma unroll	
	for (int i = 0; i < 40; i ++) {
	  if (put_idx == 0) {
	    line_in_1bit[i + 40 * (0)] = hash2addr_in_1bit[i] & 1;
	  }
	  else if (put_idx == 1) {
	    line_in_1bit[i + 40 * (1)] = hash2addr_in_1bit[i] & 1;	    
	  }
	  else if (put_idx == 2) {
	    line_in_1bit[i + 40 * (2)] = hash2addr_in_1bit[i] & 1;	    
	  }
	  else if (put_idx == 3) {
	    line_in_1bit[i + 40 * (3)] = hash2addr_in_1bit[i] & 1;	    
	  }
	  else if (put_idx == 4) {
	    line_in_1bit[i + 40 * (4)] = hash2addr_in_1bit[i] & 1;	    
	  }
	  else if (put_idx == 5) {
	    line_in_1bit[i + 40 * (5)] = hash2addr_in_1bit[i] & 1;	    
	  }
	  else if (put_idx == 6) {
	    line_in_1bit[i + 40 * (6)] = hash2addr_in_1bit[i] & 1;	    
	  }
	  else if (put_idx == 7) {
	    line_in_1bit[i + 40 * (7)] = hash2addr_in_1bit[i] & 1;	    
	  }
	  else if (put_idx == 8) {
	    line_in_1bit[i + 40 * (8)] = hash2addr_in_1bit[i] & 1;	    
	  }
	  else if (put_idx == 9) {
	    line_in_1bit[i + 40 * (9)] = hash2addr_in_1bit[i] & 1;	    
	  }
	}	
	
	// modify metadata
#pragma unroll
	for (int i = 0; i < 5; i ++) {
	  if (put_idx == 0) {
	    line_in_1bit[400 + 5 * (0) + i] = 1;
	  }
	  else if (put_idx == 1) {
	    line_in_1bit[400 + 5 * (1) + i] = 1;
	  }
	  else if (put_idx == 2) {
	    line_in_1bit[400 + 5 * (2) + i] = 1;
	  }
	  else if (put_idx == 3) {
	    line_in_1bit[400 + 5 * (3) + i] = 1;
	  }
	  else if (put_idx == 4) {
	    line_in_1bit[400 + 5 * (4) + i] = 1;
	  }
	  else if (put_idx == 5) {
	    line_in_1bit[400 + 5 * (5) + i] = 1;
	  }
	  else if (put_idx == 6) {
	    line_in_1bit[400 + 5 * (6) + i] = 1;
	  }
	  else if (put_idx == 7) {
	    line_in_1bit[400 + 5 * (7) + i] = 1;
	  }
	  else if (put_idx == 8) {
	    line_in_1bit[400 + 5 * (8) + i] = 1;
	  }
	  else if (put_idx == 9) {
	    line_in_1bit[400 + 5 * (9) + i] = 1;
	  }	  
	}

	ushort total_size = 2 + req.key_size + req.val_size;
	uchar slab_type[3];
	if (total_size <= 32) {
	  slab_type[0] = 0;
	  slab_type[1] = 0;
	  slab_type[2] = 0;	  
	}
	else if (total_size <= 64) {
	  slab_type[0] = 0;
	  slab_type[1] = 0;
	  slab_type[2] = 1;
	}
	else if (total_size <= 128) {
	  slab_type[0] = 0;
	  slab_type[1] = 1;
	  slab_type[2] = 0;	  
	}
	else if (total_size <= 256) {
	  slab_type[0] = 0;
	  slab_type[1] = 1;
	  slab_type[2] = 1;	  	  
	}
	else if (total_size <= 512) {
	  slab_type[0] = 1;
	  slab_type[1] = 0;
	  slab_type[2] = 0;	  	  	  
	}

#pragma unroll	
	for (int i = 0; i < 3; i ++) {
	  if (put_idx == 0) {
	    line_in_1bit[i + 450 + 3 * (0)] = slab_type[i] & 1;
	  }
	  else if (put_idx == 1) {
	    line_in_1bit[i + 450 + 3 * (1)] = slab_type[i] & 1;
	  }
	  else if (put_idx == 2) {
	    line_in_1bit[i + 450 + 3 * (2)] = slab_type[i] & 1;
	  }
	  else if (put_idx == 3) {
	    line_in_1bit[i + 450 + 3 * (3)] = slab_type[i] & 1;
	  }
	  else if (put_idx == 4) {
	    line_in_1bit[i + 450 + 3 * (4)] = slab_type[i] & 1;
	  }
	  else if (put_idx == 5) {
	    line_in_1bit[i + 450 + 3 * (5)] = slab_type[i] & 1;
	  }
	  else if (put_idx == 6) {
	    line_in_1bit[i + 450 + 3 * (6)] = slab_type[i] & 1;
	  }
	  else if (put_idx == 7) {
	    line_in_1bit[i + 450 + 3 * (7)] = slab_type[i] & 1;
	  }
	  else if (put_idx == 8) {
	    line_in_1bit[i + 450 + 3 * (8)] = slab_type[i] & 1;
	  }
	  else if (put_idx == 9) {
	    line_in_1bit[i + 450 + 3 * (9)] = slab_type[i] & 1;
	  }
	}
	
	uchar line_in_1byte[64];
#pragma unroll	
	for (int i = 0; i < 64; i ++) {
	  line_in_1byte[i] =
	    (line_in_1bit[0 + (i << 3)] << 7) |
	    (line_in_1bit[1 + (i << 3)] << 6) |
	    (line_in_1bit[2 + (i << 3)] << 5) |
	    (line_in_1bit[3 + (i << 3)] << 4) |
	    (line_in_1bit[4 + (i << 3)] << 3) |
	    (line_in_1bit[5 + (i << 3)] << 2) |
	    (line_in_1bit[6 + (i << 3)] << 1) |
	    (line_in_1bit[7 + (i << 3)] << 0);	    
	}
	
	ulong line_in_1ulong[8];
#pragma unroll
	for (int i = 0; i < 8; i ++) {
	  line_in_1ulong[i] =
	    (((ulong)line_in_1byte[0 + (i << 3)]) << 56) |
	    (((ulong)line_in_1byte[1 + (i << 3)]) << 48) |
	    (((ulong)line_in_1byte[2 + (i << 3)]) << 40) |
	    (((ulong)line_in_1byte[3 + (i << 3)]) << 32) |
	    (((ulong)line_in_1byte[4 + (i << 3)]) << 24) |
	    (((ulong)line_in_1byte[5 + (i << 3)]) << 16) |
	    (((ulong)line_in_1byte[6 + (i << 3)]) << 8) |
	    (((ulong)line_in_1byte[7 + (i << 3)]) << 0);
	}
	if (put_idx <= 6) {
	  DMA_WriteReq wr_req;
	  if (!req.has_last) {
	    wr_req.req.address = ((req.hash1) << 6) + line_start_addr;
	  }
	  else {
	    wr_req.req.address = ((req.hash1) << 5) + slab_start_addr;
	  }
	  wr_req.req.size = 64;
	  wr_req.req.data.x = line_in_1ulong[0];
	  wr_req.req.data.y = line_in_1ulong[1];
	  wr_req.req.data.z = line_in_1ulong[2];
	  wr_req.req.data.w = line_in_1ulong[3];
	  should_write_hashtable_put_offline_update_line_dma_wr_req = true;
	  val_hashtable_put_offline_update_line_dma_wr_req.x = wr_req.raw;

	  wr_req.req.address = 0;
	  wr_req.req.data.x = line_in_1ulong[4];
	  wr_req.req.data.y = line_in_1ulong[5];
	  wr_req.req.data.z = line_in_1ulong[6];
	  wr_req.req.data.w = line_in_1ulong[7];
	  wr_req.req.size = 0;
	  val_hashtable_put_offline_update_line_dma_wr_req.y = wr_req.raw;
	  val_hashtable_put_offline_update_line_dma_wr_req.valid2 = true;
	}
	else {
	  DMA_WriteReq wr_req;
	  if (!req.has_last) {
	    wr_req.req.address = (req.hash1 << 6) + line_start_addr + 32;
	  }
	  else {
	    wr_req.req.address = (req.hash1 << 5) + slab_start_addr + 32;
	  }	  
	  wr_req.req.size = 32;
	  wr_req.req.data.x = line_in_1ulong[4];
	  wr_req.req.data.y = line_in_1ulong[5];
	  wr_req.req.data.z = line_in_1ulong[6];
	  wr_req.req.data.w = line_in_1ulong[7];
	 	  
	  should_write_hashtable_put_offline_update_line_dma_wr_req = true;
	  val_hashtable_put_offline_update_line_dma_wr_req.x = wr_req.raw;
	  ulong8 empty;
	  empty.hi.x = empty.hi.y = empty.hi.z = empty.hi.w =
	    empty.lo.x = empty.lo.y = empty.lo.z = empty.lo.w = 0;
	  val_hashtable_put_offline_update_line_dma_wr_req.y = empty;
	  val_hashtable_put_offline_update_line_dma_wr_req.valid2 = false;
	}
      }
    }

    if (!last_finished) {
      if (first_write) {
	first_write = false;
	is_valid_put_offline_handler = false;
	uchar data_in_byte[32];
      
	data_in_byte[0] = (req.key_size << 3) | (req.val_size >> 8);
	data_in_byte[1] = req.val_size & 0xFF;

#pragma unroll
	for (int i = 0; i < 8; i ++) {
	  data_in_byte[2 + i] = (req.key.x >> (56 - (i << 3))) & 0xFF;
	  data_in_byte[2 + i + 8] = (req.key.y >> (56 - (i << 3))) & 0xFF;
	  data_in_byte[2 + i + 16] = (req.key.z >> (56 - (i << 3))) & 0xFF;
	  if (2 + i + 24 < 32) {
	    data_in_byte[2 + i + 24] = (req.key.w >> (56 - (i << 3))) & 0xFF;
	  }
	}
#pragma unroll
	for (int i = 0; i < 8; i ++) {
	  val_in_byte[i] = (req.val.x >> (56 - (i << 3))) & 0xFF;
	  val_in_byte[i + 8] = (req.val.y >> (56 - (i << 3))) & 0xFF;
	  val_in_byte[i + 16] = (req.val.z >> (56 - (i << 3))) & 0xFF;
	  val_in_byte[i + 24] = (req.val.w >> (56 - (i << 3))) & 0xFF;	
	}
      
#pragma unroll
	for (int i = 0; i < 32; i ++) {
	  if (i >= 2 + req.key_size) {
	    data_in_byte[i] = val_in_byte[i - 2 - req.key_size];
	  }
	}

	ulong data_in_ulong[4];
#pragma unroll
	for (int i = 0; i < 4; i ++) {
	  data_in_ulong[i] =
	    (((ulong)data_in_byte[(i << 3)]) << 56) |
	    (((ulong)data_in_byte[(i << 3) + 1]) << 48) |
	    (((ulong)data_in_byte[(i << 3) + 2]) << 40) |
	    (((ulong)data_in_byte[(i << 3) + 3]) << 32) |
	    (((ulong)data_in_byte[(i << 3) + 4]) << 24) |
	    (((ulong)data_in_byte[(i << 3) + 5]) << 16) |
	    (((ulong)data_in_byte[(i << 3) + 6]) << 8) |
	    (((ulong)data_in_byte[(i << 3) + 7]) << 0);
	}

	DMA_WriteReq wr_req;
	wr_req.req.address = slab_grant_addr;
	uchar total_size = 2 + req.key_size + req.val_size;
	if (total_size & 31) {
	  wr_req.req.size = ((total_size >> 5) << 5) + 32;
	}
	else {
	  wr_req.req.size = total_size;
	}
	wr_req.req.data.x = data_in_ulong[0];
	wr_req.req.data.y = data_in_ulong[1];
	wr_req.req.data.z = data_in_ulong[2];
	wr_req.req.data.w = data_in_ulong[3];
	
	val_hashtable_put_offline_update_slab_dma_wr_req.x = wr_req.raw;
	should_write_hashtable_put_offline_update_slab_dma_wr_req = true;

	val_size_left =
	  (val_size_left > (32 - 2 - req.key_size)) ?
	  (val_size_left - (32 - 2 - req.key_size)) :
	  0;
      
	if (val_size_left > 0 && val_size_left <= 2 + req.key_size) {
	  val_size_left = 0;	
#pragma unroll      
	  for (int i = 0; i < 32; i ++) {
	    if (i < 2 + req.key_size) {
	      data_in_byte[i] = val_in_byte[i + 30 - req.key_size];
	    }
	    else {
	      data_in_byte[i] = 0;
	    }
	  }
#pragma unroll
	  for (int i = 0; i < 4; i ++) {
	    data_in_ulong[i] =
	      (((ulong)data_in_byte[(i << 3)]) << 56) |
	      (((ulong)data_in_byte[(i << 3) + 1]) << 48) |
	      (((ulong)data_in_byte[(i << 3) + 2]) << 40) |
	      (((ulong)data_in_byte[(i << 3) + 3]) << 32) |
	      (((ulong)data_in_byte[(i << 3) + 4]) << 24) |
	      (((ulong)data_in_byte[(i << 3) + 5]) << 16) |
	      (((ulong)data_in_byte[(i << 3) + 6]) << 8) |
	      (((ulong)data_in_byte[(i << 3) + 7]) << 0);
	  }
	  wr_req.req.address = 0;
	  wr_req.req.size = 0;
	  wr_req.req.data.x = data_in_ulong[0];
	  wr_req.req.data.y = data_in_ulong[1];
	  wr_req.req.data.z = data_in_ulong[2];
	  wr_req.req.data.w = data_in_ulong[3];
	  val_hashtable_put_offline_update_slab_dma_wr_req.y = wr_req.raw;
	  val_hashtable_put_offline_update_slab_dma_wr_req.valid2 = true;
	}
	else {
	  val_hashtable_put_offline_update_slab_dma_wr_req.valid2 = false;
	  ulong8 empty;
	  empty.lo.x = empty.lo.y = empty.lo.z = empty.lo.w =
	    empty.hi.x = empty.lo.y = empty.lo.z = empty.lo.w = 0;
	  val_hashtable_put_offline_update_slab_dma_wr_req.y = empty;
	}
      
	if (val_size_left == 0) {
	  last_finished = true;
	  PutRes res;
	  res.found = true;
	  res.key_size = req.key_size;
	  res.net_meta = req.net_meta;
	  res.key = req.key;
	  should_write_put_offline_res = true;
	  val_write_put_offline_res = res;
	}
      }
      else {
	uchar data_in_byte[32];
#pragma unroll      
	for (int i = 0; i < 32; i ++) {
	  if (i < 2 + req.key_size) {
	    data_in_byte[i] = val_in_byte[i + 30 - req.key_size];
	  }
	  else {
	    data_in_byte[i] = 0;
	  }
	}
	is_valid_put_offline_handler = false;
	PutReq req = show_ahead_put_offline_handler.req;
#pragma unroll
	for (int i = 0; i < 8; i ++) {
	  val_in_byte[i] = (req.val.x >> (56 - (i << 3))) & 0xFF;
	  val_in_byte[i + 8] = (req.val.y >> (56 - (i << 3))) & 0xFF;
	  val_in_byte[i + 16] = (req.val.z >> (56 - (i << 3))) & 0xFF;
	  val_in_byte[i + 24] = (req.val.w >> (56 - (i << 3))) & 0xFF;	
	}
#pragma unroll      
	for (int i = 0; i < 32; i ++) {
	  if (i >= 2 + req.key_size) {
	    data_in_byte[i] = val_in_byte[i - (2 + req.key_size)];
	  }
	}
	ulong data_in_ulong[4];
#pragma unroll
	for (int i = 0; i < 4; i ++) {
	  data_in_ulong[i] =
	    (((ulong)data_in_byte[(i << 3)]) << 56) |
	    (((ulong)data_in_byte[(i << 3) + 1]) << 48) |
	    (((ulong)data_in_byte[(i << 3) + 2]) << 40) |
	    (((ulong)data_in_byte[(i << 3) + 3]) << 32) |
	    (((ulong)data_in_byte[(i << 3) + 4]) << 24) |
	    (((ulong)data_in_byte[(i << 3) + 5]) << 16) |
	    (((ulong)data_in_byte[(i << 3) + 6]) << 8) |
	    (((ulong)data_in_byte[(i << 3) + 7]) << 0);
	}
	DMA_WriteReq wr_req;
	wr_req.req.address = 0;
	wr_req.req.size = 0; 
	wr_req.req.data.x = data_in_ulong[0];
	wr_req.req.data.y = data_in_ulong[1];
	wr_req.req.data.z = data_in_ulong[2];
	wr_req.req.data.w = data_in_ulong[3];
	
	val_hashtable_put_offline_update_slab_dma_wr_req.x = wr_req.raw;
	should_write_hashtable_put_offline_update_slab_dma_wr_req = true;
      
	if (val_size_left > 32) {
	  val_size_left -= 32;
	}
	else {
	  val_size_left = 0;
	}

	if (val_size_left > 0 && val_size_left <= 2 + req.key_size) {
	  val_size_left = 0;	
#pragma unroll      
	  for (int i = 0; i < 32; i ++) {
	    if (i < 2 + req.key_size) {
	      data_in_byte[i] = val_in_byte[i + 30 - req.key_size];
	    }
	    else {
	      data_in_byte[i] = 0;
	    }
	  }
#pragma unroll
	  for (int i = 0; i < 4; i ++) {
	    data_in_ulong[i] =
	      (((ulong)data_in_byte[(i << 3)]) << 56) |
	      (((ulong)data_in_byte[(i << 3) + 1]) << 48) |
	      (((ulong)data_in_byte[(i << 3) + 2]) << 40) |
	      (((ulong)data_in_byte[(i << 3) + 3]) << 32) |
	      (((ulong)data_in_byte[(i << 3) + 4]) << 24) |
	      (((ulong)data_in_byte[(i << 3) + 5]) << 16) |
	      (((ulong)data_in_byte[(i << 3) + 6]) << 8) |
	      (((ulong)data_in_byte[(i << 3) + 7]) << 0);
	  }
	  wr_req.req.address = 0;
	  wr_req.req.size = 0;
	  wr_req.req.data.x = data_in_ulong[0];
	  wr_req.req.data.y = data_in_ulong[1];
	  wr_req.req.data.z = data_in_ulong[2];
	  wr_req.req.data.w = data_in_ulong[3];
	  	  
	  val_hashtable_put_offline_update_slab_dma_wr_req.y = wr_req.raw;
	  val_hashtable_put_offline_update_slab_dma_wr_req.valid2 = true;
	}
	else {
	  val_hashtable_put_offline_update_slab_dma_wr_req.valid2 = false;
	  ulong8 empty;
	  empty.lo.x = empty.lo.y = empty.lo.z = empty.lo.w =
	    empty.hi.x = empty.hi.y = empty.hi.z = empty.hi.w = 0;
	  val_hashtable_put_offline_update_slab_dma_wr_req.y = empty;
	}
      
	if (val_size_left == 0) {
	  last_finished = true;
	  PutRes res;
	  res.found = true;
	  res.key_size = req.key_size;
	  res.net_meta = req.net_meta;
	  res.key = req.key;
	  should_write_put_offline_res = true;
	  val_write_put_offline_res = res;
	}
      }
    }
   
    bool dummy;
    if (should_write_hashtable_put_offline_update_line_dma_wr_req) {
      DMA_WriteReq req;
      req.raw = val_hashtable_put_offline_update_line_dma_wr_req.x;
      dummy = write_channel_nb_altera(hashtable_put_offline_update_line_dma_wr_req_double,
				      val_hashtable_put_offline_update_line_dma_wr_req);
      assert(dummy);
    }

    if (should_write_hashtable_put_offline_update_slab_dma_wr_req) {
      dummy = write_channel_nb_altera(hashtable_put_offline_update_slab_dma_wr_req_double,
				      val_hashtable_put_offline_update_slab_dma_wr_req);
      assert(dummy);
    }

    if (should_write_put_offline_res) {
      dummy = write_channel_nb_altera(put_offline_res, val_write_put_offline_res);
      assert(dummy);
    }
    
  }
}
