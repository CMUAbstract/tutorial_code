#include "common/fft.h"

void task_fft1d();
TASK(task_fft1d);

void task_fft1d() {
#ifdef TEST_INNER
	P1OUT |= BIT1;
	P1DIR |= BIT1;
	P1OUT &= ~BIT1;
#endif
	// Point ptrs to correct matrices
	mat_t *dest_real_ptr = dest_real;
	mat_t *dest_imag_ptr = dest_imag;
	mat_t *src_real_ptr = inter_real;
	mat_t *src_imag_ptr = inter_imag;
	//printf("Mat start for row=%i col=%i, num_elem=%i, stride=%i\r\n",row_idx,
	//col_idx, num_elements, stride);
	//MAT_DUMP(inter_real,0);
	//MAT_DUMP(inter_imag,0);
	// Not sure why we need to transform here, but whatever
	fixed *src_real_ptr_base = MAT_PTR(src_real_ptr, row_idx, col_idx);
	fixed *src_imag_ptr_base = MAT_PTR(src_imag_ptr, row_idx, col_idx);
	fixed *dest_real_ptr_base = MAT_PTR(dest_real_ptr, row_idx, col_idx);
	fixed *dest_imag_ptr_base = MAT_PTR(dest_imag_ptr, row_idx, col_idx);
	// Copy src to dest
	for(uint16_t i = 0; i < num_elements * stride; i += stride) {
		*(dest_real_ptr_base + i) = *(src_real_ptr_base + i);
		*(dest_imag_ptr_base + i) = *(src_imag_ptr_base + i);
	}
	// Actual meat of the operation
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
			// Write to destination
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
			// More updates to destination
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
	//printf("Mat dump for iglob=%i jglob=%i\r\n",i_glob, j_glob);
	//MAT_DUMP(dest_real,0);
#ifdef TEST_INNER
	P1OUT |= BIT2;
	P1DIR |= BIT2;
	P1OUT &= ~BIT2;
#endif
	TRANSITION_TO(task_compute);
}
