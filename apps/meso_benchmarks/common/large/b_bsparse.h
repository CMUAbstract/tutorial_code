#ifndef B_BSPARSE_H
#define B_BSPARSE_H

#include <libfixed/fixed.h>

#if BLOCK_SIZE == 16

#define B_BSPARSE_LEN 188

__nv fixed b_bsparse[188] = {
	F_LIT(-3), F_LIT(-3), F_LIT(-5), F_LIT(-3), F_LIT(3),  F_LIT(4),  F_LIT(-4),
	F_LIT(4),  F_LIT(-5), F_LIT(-5), F_LIT(3),  F_LIT(4),  F_LIT(4),  F_LIT(4),
	F_LIT(-5), F_LIT(-5), F_LIT(-5), F_LIT(3),  F_LIT(2),  F_LIT(2),  F_LIT(-2),
	F_LIT(-4), F_LIT(4),  F_LIT(3),  F_LIT(-3), F_LIT(-4), F_LIT(-4), F_LIT(4),
	F_LIT(-4), F_LIT(-5), F_LIT(3),  F_LIT(-4), F_LIT(-3), F_LIT(3),  F_LIT(-4),
	F_LIT(-5), F_LIT(2),  F_LIT(-2), F_LIT(3),  F_LIT(-2), F_LIT(-4), F_LIT(3),
	F_LIT(3),  F_LIT(2),  F_LIT(3),  F_LIT(2),  F_LIT(4),  F_LIT(-5), F_LIT(4),
	F_LIT(-2), F_LIT(-5), F_LIT(3),  F_LIT(-2), F_LIT(4),  F_LIT(-2), F_LIT(2),
	F_LIT(-5), F_LIT(2),  F_LIT(3),  F_LIT(-3), F_LIT(2),  F_LIT(-2), F_LIT(4),
	F_LIT(-5), F_LIT(-5), F_LIT(-5), F_LIT(-2), F_LIT(-2), F_LIT(3),  F_LIT(-5),
	F_LIT(2),  F_LIT(-3), F_LIT(4),  F_LIT(-2), F_LIT(3),  F_LIT(-4), F_LIT(3),
	F_LIT(-5), F_LIT(-2), F_LIT(3),  F_LIT(-5), F_LIT(-4), F_LIT(2),  F_LIT(-4),
	F_LIT(-4), F_LIT(4),  F_LIT(-3), F_LIT(-2), F_LIT(4),  F_LIT(-5), F_LIT(4),
	F_LIT(4),  F_LIT(2),  F_LIT(-5), F_LIT(-2), F_LIT(-4), F_LIT(-3), F_LIT(3),
	F_LIT(3),  F_LIT(-2), F_LIT(-2), F_LIT(-5), F_LIT(3),  F_LIT(2),  F_LIT(2),
	F_LIT(-3), F_LIT(-5), F_LIT(2),  F_LIT(-5), F_LIT(-2), F_LIT(2),  F_LIT(3),
	F_LIT(-4), F_LIT(-5), F_LIT(-3), F_LIT(2),  F_LIT(-5), F_LIT(-3), F_LIT(-3),
	F_LIT(3),  F_LIT(3),  F_LIT(2),  F_LIT(-2), F_LIT(-5), F_LIT(3),  F_LIT(2),
	F_LIT(-2), F_LIT(-4), F_LIT(-4), F_LIT(-3), F_LIT(2),  F_LIT(-2), F_LIT(2),
	F_LIT(4),  F_LIT(3),  F_LIT(2),  F_LIT(-4), F_LIT(-2), F_LIT(3),  F_LIT(-3),
	F_LIT(4),  F_LIT(-4), F_LIT(-5), F_LIT(4),  F_LIT(-2), F_LIT(3),  F_LIT(-4),
	F_LIT(2),  F_LIT(-2), F_LIT(-2), F_LIT(-4), F_LIT(3),  F_LIT(3),  F_LIT(3),
	F_LIT(-2), F_LIT(-5), F_LIT(-4), F_LIT(2),  F_LIT(4),  F_LIT(-5), F_LIT(-2),
	F_LIT(-4), F_LIT(-2), F_LIT(2),  F_LIT(3),  F_LIT(-4), F_LIT(3),  F_LIT(-2),
	F_LIT(3),  F_LIT(-3), F_LIT(2),  F_LIT(-3), F_LIT(3),  F_LIT(-4), F_LIT(4),
	F_LIT(4),  F_LIT(3),  F_LIT(-5), F_LIT(-4), F_LIT(-5), F_LIT(-3), F_LIT(-2),
	F_LIT(-3), F_LIT(-2), F_LIT(-3), F_LIT(-2), F_LIT(3),  F_LIT(3)};

__nv uint16_t b_bsparse_offsets[188] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

__nv uint16_t b_bsparse_sizes[272] = {
	0,  1,  1,  1,  2,  3,  4,  5,  6,  7,  8,  8,  9,  9,  10, 10, 11, 0,  1,
	2,  3,  4,  5,  5,  6,  7,  8,  9,  9,  10, 10, 10, 11, 12, 0,  1,  2,  3,
	4,  5,  5,  5,  6,  7,  8,  8,  8,  9,  10, 11, 12, 0,  1,  2,  3,  4,  4,
	5,  6,  7,  8,  8,  9,  9,  10, 11, 11, 11, 0,  1,  1,  2,  2,  3,  4,  5,
	6,  7,  8,  9,  10, 11, 12, 13, 14, 0,  1,  2,  3,  4,  5,  6,  7,  7,  8,
	9,  10, 11, 12, 13, 14, 14, 0,  0,  0,  1,  2,  3,  4,  4,  4,  5,  6,  6,
	7,  7,  7,  8,  8,  0,  1,  1,  2,  3,  3,  4,  5,  5,  6,  7,  7,  8,  9,
	10, 11, 12, 0,  1,  2,  3,  3,  4,  5,  5,  6,  7,  8,  9,  9,  9,  10, 11,
	12, 0,  0,  1,  2,  3,  3,  4,  4,  4,  5,  6,  6,  7,  8,  9,  9,  9,  0,
	0,  1,  1,  1,  2,  3,  4,  5,  5,  6,  7,  8,  9,  10, 11, 11, 0,  0,  1,
	2,  2,  3,  4,  5,  6,  6,  7,  8,  9,  10, 11, 12, 13, 0,  1,  1,  2,  2,
	3,  4,  5,  6,  7,  8,  8,  9,  10, 10, 10, 11, 0,  1,  1,  2,  3,  4,  5,
	6,  7,  8,  9,  10, 11, 12, 13, 13, 14, 0,  1,  2,  3,  4,  5,  6,  7,  8,
	9,  9,  10, 11, 11, 12, 12, 13, 0,  1,  2,  3,  4,  5,  6,  6,  7,  8,  8,
	8,  9,  10, 11, 11, 11};

#elif BLOCK_SIZE == 32

#define B_BSPARSE_LEN 188

__nv fixed b_bsparse[188] = {
	F_LIT(-3), F_LIT(-3), F_LIT(-5), F_LIT(-3), F_LIT(3),  F_LIT(4),  F_LIT(-4),
	F_LIT(4),  F_LIT(-5), F_LIT(-5), F_LIT(3),  F_LIT(4),  F_LIT(4),  F_LIT(4),
	F_LIT(-5), F_LIT(-5), F_LIT(-5), F_LIT(3),  F_LIT(2),  F_LIT(2),  F_LIT(-2),
	F_LIT(-4), F_LIT(4),  F_LIT(3),  F_LIT(-3), F_LIT(-4), F_LIT(-4), F_LIT(4),
	F_LIT(-4), F_LIT(-5), F_LIT(3),  F_LIT(-4), F_LIT(-3), F_LIT(3),  F_LIT(-4),
	F_LIT(-5), F_LIT(2),  F_LIT(-2), F_LIT(3),  F_LIT(-2), F_LIT(-4), F_LIT(3),
	F_LIT(3),  F_LIT(2),  F_LIT(3),  F_LIT(2),  F_LIT(4),  F_LIT(-5), F_LIT(4),
	F_LIT(-2), F_LIT(-5), F_LIT(3),  F_LIT(-2), F_LIT(4),  F_LIT(-2), F_LIT(2),
	F_LIT(-5), F_LIT(2),  F_LIT(3),  F_LIT(-3), F_LIT(2),  F_LIT(-2), F_LIT(4),
	F_LIT(-5), F_LIT(-5), F_LIT(-5), F_LIT(-2), F_LIT(-2), F_LIT(3),  F_LIT(-5),
	F_LIT(2),  F_LIT(-3), F_LIT(4),  F_LIT(-2), F_LIT(3),  F_LIT(-4), F_LIT(3),
	F_LIT(-5), F_LIT(-2), F_LIT(3),  F_LIT(-5), F_LIT(-4), F_LIT(2),  F_LIT(-4),
	F_LIT(-4), F_LIT(4),  F_LIT(-3), F_LIT(-2), F_LIT(4),  F_LIT(-5), F_LIT(4),
	F_LIT(4),  F_LIT(2),  F_LIT(-5), F_LIT(-2), F_LIT(-4), F_LIT(-3), F_LIT(3),
	F_LIT(3),  F_LIT(-2), F_LIT(-2), F_LIT(-5), F_LIT(3),  F_LIT(2),  F_LIT(2),
	F_LIT(-3), F_LIT(-5), F_LIT(2),  F_LIT(-5), F_LIT(-2), F_LIT(2),  F_LIT(3),
	F_LIT(-4), F_LIT(-5), F_LIT(-3), F_LIT(2),  F_LIT(-5), F_LIT(-3), F_LIT(-3),
	F_LIT(3),  F_LIT(3),  F_LIT(2),  F_LIT(-2), F_LIT(-5), F_LIT(3),  F_LIT(2),
	F_LIT(-2), F_LIT(-4), F_LIT(-4), F_LIT(-3), F_LIT(2),  F_LIT(-2), F_LIT(2),
	F_LIT(4),  F_LIT(3),  F_LIT(2),  F_LIT(-4), F_LIT(-2), F_LIT(3),  F_LIT(-3),
	F_LIT(4),  F_LIT(-4), F_LIT(-5), F_LIT(4),  F_LIT(-2), F_LIT(3),  F_LIT(-4),
	F_LIT(2),  F_LIT(-2), F_LIT(-2), F_LIT(-4), F_LIT(3),  F_LIT(3),  F_LIT(3),
	F_LIT(-2), F_LIT(-5), F_LIT(-4), F_LIT(2),  F_LIT(4),  F_LIT(-5), F_LIT(-2),
	F_LIT(-4), F_LIT(-2), F_LIT(2),  F_LIT(3),  F_LIT(-4), F_LIT(3),  F_LIT(-2),
	F_LIT(3),  F_LIT(-3), F_LIT(2),  F_LIT(-3), F_LIT(3),  F_LIT(-4), F_LIT(4),
	F_LIT(4),  F_LIT(3),  F_LIT(-5), F_LIT(-4), F_LIT(-5), F_LIT(-3), F_LIT(-2),
	F_LIT(-3), F_LIT(-2), F_LIT(-3), F_LIT(-2), F_LIT(3),  F_LIT(3)};

__nv uint16_t b_bsparse_offsets[188] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

__nv uint16_t b_bsparse_sizes[264] = {
	0,  1,  1,  1,  2,  3,  4,  5,  6,  7,  8,  8,  9,  9,  10, 10, 11, 12, 13,
	14, 15, 16, 16, 17, 18, 19, 20, 20, 21, 21, 21, 22, 23, 0,  1,  2,  3,  4,
	5,  5,  5,  6,  7,  8,  8,  8,  9,  10, 11, 12, 13, 14, 15, 16, 16, 17, 18,
	19, 20, 20, 21, 21, 22, 23, 23, 23, 0,  1,  1,  2,  2,  3,  4,  5,  6,  7,
	8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 21, 22, 23, 24, 25,
	26, 27, 28, 28, 0,  0,  0,  1,  2,  3,  4,  4,  4,  5,  6,  6,  7,  7,  7,
	8,  8,  9,  9,  10, 11, 11, 12, 13, 13, 14, 15, 15, 16, 17, 18, 19, 20, 0,
	1,  2,  3,  3,  4,  5,  5,  6,  7,  8,  9,  9,  9,  10, 11, 12, 12, 13, 14,
	15, 15, 16, 16, 16, 17, 18, 18, 19, 20, 21, 21, 21, 0,  0,  1,  1,  1,  2,
	3,  4,  5,  5,  6,  7,  8,  9,  10, 11, 11, 11, 12, 13, 13, 14, 15, 16, 17,
	17, 18, 19, 20, 21, 22, 23, 24, 0,  1,  1,  2,  2,  3,  4,  5,  6,  7,  8,
	8,  9,  10, 10, 10, 11, 12, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	24, 24, 25, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  9,  10, 11, 11, 12, 12,
	13, 14, 15, 16, 17, 18, 19, 19, 20, 21, 21, 21, 22, 23, 24, 24, 24};

#elif BLOCK_SIZE == 64

#define B_BSPARSE_LEN 188

__nv fixed b_bsparse[188] = {
	F_LIT(-3), F_LIT(-3), F_LIT(-5), F_LIT(-3), F_LIT(3),  F_LIT(4),  F_LIT(-4),
	F_LIT(4),  F_LIT(-5), F_LIT(-5), F_LIT(3),  F_LIT(4),  F_LIT(4),  F_LIT(4),
	F_LIT(-5), F_LIT(-5), F_LIT(-5), F_LIT(3),  F_LIT(2),  F_LIT(2),  F_LIT(-2),
	F_LIT(-4), F_LIT(4),  F_LIT(3),  F_LIT(-3), F_LIT(-4), F_LIT(-4), F_LIT(4),
	F_LIT(-4), F_LIT(-5), F_LIT(3),  F_LIT(-4), F_LIT(-3), F_LIT(3),  F_LIT(-4),
	F_LIT(-5), F_LIT(2),  F_LIT(-2), F_LIT(3),  F_LIT(-2), F_LIT(-4), F_LIT(3),
	F_LIT(3),  F_LIT(2),  F_LIT(3),  F_LIT(2),  F_LIT(4),  F_LIT(-5), F_LIT(4),
	F_LIT(-2), F_LIT(-5), F_LIT(3),  F_LIT(-2), F_LIT(4),  F_LIT(-2), F_LIT(2),
	F_LIT(-5), F_LIT(2),  F_LIT(3),  F_LIT(-3), F_LIT(2),  F_LIT(-2), F_LIT(4),
	F_LIT(-5), F_LIT(-5), F_LIT(-5), F_LIT(-2), F_LIT(-2), F_LIT(3),  F_LIT(-5),
	F_LIT(2),  F_LIT(-3), F_LIT(4),  F_LIT(-2), F_LIT(3),  F_LIT(-4), F_LIT(3),
	F_LIT(-5), F_LIT(-2), F_LIT(3),  F_LIT(-5), F_LIT(-4), F_LIT(2),  F_LIT(-4),
	F_LIT(-4), F_LIT(4),  F_LIT(-3), F_LIT(-2), F_LIT(4),  F_LIT(-5), F_LIT(4),
	F_LIT(4),  F_LIT(2),  F_LIT(-5), F_LIT(-2), F_LIT(-4), F_LIT(-3), F_LIT(3),
	F_LIT(3),  F_LIT(-2), F_LIT(-2), F_LIT(-5), F_LIT(3),  F_LIT(2),  F_LIT(2),
	F_LIT(-3), F_LIT(-5), F_LIT(2),  F_LIT(-5), F_LIT(-2), F_LIT(2),  F_LIT(3),
	F_LIT(-4), F_LIT(-5), F_LIT(-3), F_LIT(2),  F_LIT(-5), F_LIT(-3), F_LIT(-3),
	F_LIT(3),  F_LIT(3),  F_LIT(2),  F_LIT(-2), F_LIT(-5), F_LIT(3),  F_LIT(2),
	F_LIT(-2), F_LIT(-4), F_LIT(-4), F_LIT(-3), F_LIT(2),  F_LIT(-2), F_LIT(2),
	F_LIT(4),  F_LIT(3),  F_LIT(2),  F_LIT(-4), F_LIT(-2), F_LIT(3),  F_LIT(-3),
	F_LIT(4),  F_LIT(-4), F_LIT(-5), F_LIT(4),  F_LIT(-2), F_LIT(3),  F_LIT(-4),
	F_LIT(2),  F_LIT(-2), F_LIT(-2), F_LIT(-4), F_LIT(3),  F_LIT(3),  F_LIT(3),
	F_LIT(-2), F_LIT(-5), F_LIT(-4), F_LIT(2),  F_LIT(4),  F_LIT(-5), F_LIT(-2),
	F_LIT(-4), F_LIT(-2), F_LIT(2),  F_LIT(3),  F_LIT(-4), F_LIT(3),  F_LIT(-2),
	F_LIT(3),  F_LIT(-3), F_LIT(2),  F_LIT(-3), F_LIT(3),  F_LIT(-4), F_LIT(4),
	F_LIT(4),  F_LIT(3),  F_LIT(-5), F_LIT(-4), F_LIT(-5), F_LIT(-3), F_LIT(-2),
	F_LIT(-3), F_LIT(-2), F_LIT(-3), F_LIT(-2), F_LIT(3),  F_LIT(3)};

__nv uint16_t b_bsparse_offsets[188] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

__nv uint16_t b_bsparse_sizes[260] = {
	0,  1,  1,  1,  2,  3,  4,  5,  6,  7,  8,  8,  9,  9,  10, 10, 11, 12, 13,
	14, 15, 16, 16, 17, 18, 19, 20, 20, 21, 21, 21, 22, 23, 24, 25, 26, 27, 28,
	28, 28, 29, 30, 31, 31, 31, 32, 33, 34, 35, 36, 37, 38, 39, 39, 40, 41, 42,
	43, 43, 44, 44, 45, 46, 46, 46, 0,  1,  1,  2,  2,  3,  4,  5,  6,  7,  8,
	9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 21, 22, 23, 24, 25, 26,
	27, 28, 28, 28, 28, 29, 30, 31, 32, 32, 32, 33, 34, 34, 35, 35, 35, 36, 36,
	37, 37, 38, 39, 39, 40, 41, 41, 42, 43, 43, 44, 45, 46, 47, 48, 0,  1,  2,
	3,  3,  4,  5,  5,  6,  7,  8,  9,  9,  9,  10, 11, 12, 12, 13, 14, 15, 15,
	16, 16, 16, 17, 18, 18, 19, 20, 21, 21, 21, 21, 22, 22, 22, 23, 24, 25, 26,
	26, 27, 28, 29, 30, 31, 32, 32, 32, 33, 34, 34, 35, 36, 37, 38, 38, 39, 40,
	41, 42, 43, 44, 45, 0,  1,  1,  2,  2,  3,  4,  5,  6,  7,  8,  8,  9,  10,
	10, 10, 11, 12, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 24, 25,
	26, 27, 28, 29, 30, 31, 32, 33, 34, 34, 35, 36, 36, 37, 37, 38, 39, 40, 41,
	42, 43, 44, 44, 45, 46, 46, 46, 47, 48, 49, 49, 49};

#endif

#endif