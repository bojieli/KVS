_CL_VOID
slab_return() {
#define slab_size(i) (1 << ((i) + SLAB_MIN_SIZE_LOG2))
  ulong4 current_return_table_to_be_put[SLAB_BIN_COUNT];
  enum Table_Head_State current_return_table_to_be_put_state[SLAB_BIN_COUNT];
  ulong slab_start_addr;

#pragma unroll
  for (int i = 0; i < SLAB_BIN_COUNT; i ++) {
    current_return_table_to_be_put[i].x = 0;
    current_return_table_to_be_put[i].y = 0;
    current_return_table_to_be_put[i].z = 0;
    current_return_table_to_be_put[i].w = 0;
    current_return_table_to_be_put_state[i] = EMPTY;
  }

  slab_start_addr = read_channel_altera(init_slab_return);
  
  while (1) {   
    SlabReturn slabReturn;
    ClSignal out_signal;
    bool read_slab_return_req;
    slabReturn = read_channel_nb_altera(slab_return_req, &read_slab_return_req);

    if (read_slab_return_req) {
      ushort slab_return_size;
      ulong slab_return_addr;

      slab_return_size = slabReturn.slab_size;
      slab_return_addr = (slabReturn.slab_addr - slab_start_addr) >> 5;
      
#pragma unroll
      for (int i = 0; i < SLAB_BIN_COUNT; i ++) {
	if (slab_return_size <= slab_size(i) && (i == 0 || slab_return_size > slab_size(i - 1))) {
	  if (current_return_table_to_be_put_state[i] == EMPTY) {
	    current_return_table_to_be_put_state[i] = PART0;
	    current_return_table_to_be_put[i].x |= (slab_return_addr) << 2;
	  }
	  else if (current_return_table_to_be_put_state[i] == PART0) {
	    current_return_table_to_be_put_state[i] = PART1;
	    current_return_table_to_be_put[i].x |= (slab_return_addr << 32) << 2;
	  }
	  else if (current_return_table_to_be_put_state[i] == PART1) {
	    current_return_table_to_be_put_state[i] = PART2;
	    current_return_table_to_be_put[i].y |= (slab_return_addr) << 2;
	  }
	  else if (current_return_table_to_be_put_state[i] == PART2) {
	    current_return_table_to_be_put_state[i] = PART3;
	    current_return_table_to_be_put[i].y |= (slab_return_addr << 32) << 2;
	  }
	  else if (current_return_table_to_be_put_state[i] == PART3) {
	    current_return_table_to_be_put_state[i] = PART4;
	    current_return_table_to_be_put[i].z |= (slab_return_addr) << 2;
	  }
	  else if (current_return_table_to_be_put_state[i] == PART4) {
	    current_return_table_to_be_put_state[i] = PART5;
	    current_return_table_to_be_put[i].z |= (slab_return_addr << 32) << 2;
	  }
	  else if (current_return_table_to_be_put_state[i] == PART5) {
	    current_return_table_to_be_put_state[i] = PART6;
	    current_return_table_to_be_put[i].w |= (slab_return_addr) << 2;
	  }
	  else if (current_return_table_to_be_put_state[i] == PART6) {
	    current_return_table_to_be_put_state[i] = PART7;
	    current_return_table_to_be_put[i].w |= (slab_return_addr << 32) << 2;
	  }
	  else if (current_return_table_to_be_put_state[i] == PART7) {
	    bool dummy;
	    
	    dummy = write_channel_nb_altera(slab_return_table[i], current_return_table_to_be_put[i]);
	    assert(dummy);
	    dummy = write_channel_nb_altera(add_8_slab_return_table_size[i], true);
	    assert(dummy);
		
	    current_return_table_to_be_put_state[i] = PART0;
	    current_return_table_to_be_put[i].x = 0;
	    current_return_table_to_be_put[i].y = 0;
	    current_return_table_to_be_put[i].z = 0;
	    current_return_table_to_be_put[i].w = 0;
	    current_return_table_to_be_put[i].x |= (slab_return_addr) << 2;
	  }
	}
      }
    }
  }
#undef slab_size
}
