void task_dwt1d();
TASK(task_dwt1d);

#define SQRT2 F_LIT(0.707106)

void task_dwt1d() {
	fixed *src_ptr = MAT_PTR(src, row_idx, col_idx);	
	fixed *dest_ptr = MAT_PTR(dest, row_idx, col_idx);
	fixed *inter_ptr = MAT_PTR(inter, row_idx, col_idx);

	// Do a copy
	if(enable_copy) {
		for(uint16_t i = 0; i < num_elements * stride; i += stride)
			*(dest_ptr + i) = *(src_ptr + i);
	}

	uint16_t k = 1;
	while((k << 1) <= num_elements) k <<= 1;

	while(1 < k) {
		k >>= 1;
		inter_ptr = MAT_PTR(inter, row_idx, col_idx);
		dest_ptr = MAT_PTR(dest, row_idx, col_idx);
		fixed *inter_k_ptr = inter_ptr + k * stride; 
		fixed *even_dest_ptr = dest_ptr;
		fixed *odd_dest_ptr = dest_ptr + stride;
		for(uint16_t i = 0; i < k; i++) {
			*inter_ptr = F_MUL(F_ADD(*even_dest_ptr, *odd_dest_ptr), SQRT2);
			*inter_k_ptr = F_MUL(F_SUB(*even_dest_ptr, *odd_dest_ptr), SQRT2);
			inter_ptr += stride;
			inter_k_ptr += stride;
			even_dest_ptr += 2 * stride;
			odd_dest_ptr += 2 * stride;
		}

		inter_ptr = MAT_PTR(inter, row_idx, col_idx);
		// for(uint16_t j = 0; j < num_elements; j++)
		// 	printf("%d ", inter_ptr[j]);
		// printf("\n");
		for(uint16_t i = 0; i < (k << 1); i++) {
			*dest_ptr = *inter_ptr;
			dest_ptr += stride;
			inter_ptr += stride;
		}
	}

	// exit(0);
	TRANSITION_TO(task_compute);
}
