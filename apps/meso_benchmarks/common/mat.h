#if INPUT_SIZE == 1
#pragma message "Small"
#include "small/a_dense.h"
#include "small/b_dense.h"
#include "small/c_dense.h"
#include "small/a_sparse.h"
#include "small/b_sparse.h"
#include "small/c_sparse.h"
#include "small/a_bsparse.h"
#include "small/b_bsparse.h"
#define MAT_SIZE 16
#define TILE_SIZE 3
#define MAT_BLOCK_SIZE 16
#define MAT_BLOCK_COLS 1
#elif INPUT_SIZE == 2
#pragma message "Medium"
#include "medium/a_dense.h"
#include "medium/b_dense.h"
#include "medium/c_dense.h"
#include "medium/a_sparse.h"
#include "medium/b_sparse.h"
#include "medium/c_sparse.h"
#include "medium/a_bsparse.h"
#include "medium/b_bsparse.h"
#define MAT_SIZE 64
#define TILE_SIZE 5
#define MAT_BLOCK_SIZE BLOCK_SIZE
#define MAT_BLOCK_COLS MAT_SIZE / BLOCK_SIZE
#else
#pragma message "Large"
#include "large/a_dense.h"
#include "large/b_dense.h"
#include "large/c_dense.h"
#include "large/a_sparse.h"
#include "large/b_sparse.h"
#include "large/c_sparse.h"
#include "large/a_bsparse.h"
#include "large/b_bsparse.h"
#define MAT_SIZE 256
#define TILE_SIZE 5
#define MAT_BLOCK_SIZE BLOCK_SIZE
#define MAT_BLOCK_COLS MAT_SIZE / BLOCK_SIZE
#endif

__nv mat_t mat_a_dense = {
		.dims = {MAT_SIZE, MAT_SIZE}, .strides = {MAT_SIZE, 1}, .len_dims = 2, .data = a_dense,
};

__nv mat_t mat_a_sparse = {.dims = {A_SPARSE_LEN},
								.strides = {1},
								.len_dims = 1,
								.data = a_sparse,
								.sparse = {.dims = {MAT_SIZE, MAT_SIZE},
											 .len_dims = 2,
											 .offsets = a_sparse_offsets,
											 .sizes = a_sparse_sizes}};

__nv mat_t mat_a_bsparse = {.dims = {A_BSPARSE_LEN},
								.strides = {1},
								.len_dims = 1,
								.data = a_bsparse,
								.sparse = {.dims = {MAT_BLOCK_SIZE, MAT_BLOCK_COLS},
											 .len_dims = 2,
											 .offsets = a_bsparse_offsets,
											 .sizes = a_bsparse_sizes}};

__nv mat_t mat_b_dense = {
		.dims = {MAT_SIZE, 1}, .strides = {1, 1}, .len_dims = 2, .data = b_dense,
};

__nv mat_t mat_b_sparse = {.dims = {B_SPARSE_LEN},
								.strides = {1},
								.len_dims = 1,
								.data = b_sparse,
								.sparse = {.dims = {MAT_SIZE, 1},
											 .len_dims = 2,
											 .offsets = b_sparse_offsets,
											 .sizes = b_sparse_sizes}};

__nv mat_t mat_b_bsparse = {.dims = {B_BSPARSE_LEN},
								.strides = {1},
								.len_dims = 1,
								.data = b_bsparse,
								.sparse = {.dims = {MAT_BLOCK_SIZE, MAT_BLOCK_COLS},
											 .len_dims = 2,
											 .offsets = b_bsparse_offsets,
											 .sizes = b_bsparse_sizes}};

__nv mat_t mat_c_dense = {
		.dims = {1, 1, TILE_SIZE, TILE_SIZE},
		.len_dims = 4,
		.strides = {TILE_SIZE * TILE_SIZE, TILE_SIZE * TILE_SIZE, TILE_SIZE, 1},
		.data = c_dense,
};

__nv mat_t mat_c_sparse = {.dims = {C_SPARSE_LEN},
								.len_dims = 1,
								.strides = {1},
								.data = c_sparse,
								.sparse = {.dims = {1, 1, TILE_SIZE, TILE_SIZE},
											 .len_dims = 4,
											 .sizes = c_sparse_sizes,
											 .offsets = c_sparse_offsets}};

__nv fixed buf[6 * MAT_SIZE * MAT_SIZE];
__nv mat_t buf1 = {.data = buf};
__nv mat_t buf2 = {.data = buf + MAT_SIZE * MAT_SIZE};
__nv mat_t buf3 = {.data = buf + MAT_SIZE * MAT_SIZE * 2};
__nv mat_t buf4 = {.data = buf + MAT_SIZE * MAT_SIZE * 3};
__nv mat_t buf5 = {.data = buf + MAT_SIZE * MAT_SIZE * 4};
__nv mat_t buf6 = {.data = buf + MAT_SIZE * MAT_SIZE * 5};
__nv mat_t *b1 = &buf1;
__nv mat_t *b2 = &buf2;
__nv mat_t *b3 = &buf3;
__nv mat_t *b4 = &buf4;
__nv mat_t *b5 = &buf5;
__nv mat_t *b6 = &buf6;
