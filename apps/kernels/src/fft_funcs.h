#include "common/fft.h"

void task_fft1d();
TASK(task_fft1d);

void task_fft1d() {
	mat_t *dest_real_ptr = dest_real;
	mat_t *dest_imag_ptr = dest_imag;
	mat_t *src_real_ptr = inter_real;
	mat_t *src_imag_ptr = inter_imag;

	fixed *src_real_ptr_base = MAT_PTR(src_real_ptr, row_idx, col_idx);
	fixed *src_imag_ptr_base = MAT_PTR(src_imag_ptr, row_idx, col_idx);
	fixed *dest_real_ptr_base = MAT_PTR(dest_real_ptr, row_idx, col_idx);
	fixed *dest_imag_ptr_base = MAT_PTR(dest_imag_ptr, row_idx, col_idx);

	for(uint16_t i = 0; i < num_elements * stride; i += stride) {
		*(dest_real_ptr_base + i) = *(src_real_ptr_base + i);
		*(dest_imag_ptr_base + i) = *(src_imag_ptr_base + i);
	}

	for(uint16_t i = 0; i < num_elements; i++) {
		uint16_t j = 0;
		uint16_t m = i;
		uint16_t p = 1;
		while(p < num_elements) {
			j = (j << 1) + (m & 1);
			m >>= 1;
			p <<= 1;
		}
		if(j > i) { // Do a swap
			fixed tmp = *(dest_real_ptr_base + j * stride);
			*(dest_real_ptr_base + j * stride) = *(dest_real_ptr_base + i * stride);
			*(dest_real_ptr_base + i * stride) = tmp;
			tmp = *(dest_imag_ptr_base + j * stride);
			*(dest_imag_ptr_base + j * stride) = *(dest_imag_ptr_base + i * stride);
			*(dest_imag_ptr_base + i * stride) = tmp;
		}
	}

	uint16_t Ls = 1;
	while(Ls < num_elements) {
		uint16_t step = (Ls << 1);
		for(uint16_t j = 0; j < Ls; j++) {
			fixed theta = F_DIV(F_LIT(j), F_LIT(Ls));
			fixed wr = wrs[theta];
			fixed wi = F_MUL(F_LIT(-flag), wis[theta]);
			fixed *dest_real_ptr1 = dest_real_ptr_base + (j + Ls) * stride;
			fixed *dest_imag_ptr1 = dest_imag_ptr_base + (j + Ls) * stride;
			fixed *dest_real_ptr2 = dest_real_ptr_base + j * stride;
			fixed *dest_imag_ptr2 = dest_imag_ptr_base + j * stride;
			for(uint16_t k = j; k < num_elements; k += step) {
				fixed tr = F_SUB(F_MUL(wr, *dest_real_ptr1), 
					F_MUL(wi, *dest_imag_ptr1));
				fixed ti = F_ADD(F_MUL(wr, *dest_imag_ptr1), 
					F_MUL(wi, *dest_real_ptr1));
				*dest_real_ptr1 = F_SUB(*dest_real_ptr2, tr);
				*dest_imag_ptr1 = F_SUB(*dest_imag_ptr2, ti);
				*dest_real_ptr2 = F_ADD(*dest_real_ptr2, tr);
				*dest_imag_ptr2 = F_ADD(*dest_imag_ptr2, ti);
				dest_real_ptr1 += stride * step;	
				dest_imag_ptr1 += stride * step;	
				dest_real_ptr2 += stride * step;	
				dest_imag_ptr2 += stride * step;
			}
		}
		Ls <<= 1;
	}
	TRANSITION_TO(task_compute);
}
