#include "pacarana.h"

/*
  Including libalpaca here is less than ideal, but to hack together a function
  that can correctly transition for the given runtime, we need a way to access
  those struct definitions... and globals for double buffering
*/
#include "libalpaca/alpaca.h"
void __pacarana_to_alpaca_transition(void *next_task) {
	context_t *next_ctx;
	next_ctx = (curctx == &context_0 ? &context_1 : &context_0);
	next_ctx->task = &next_task;
	next_ctx->numRollback = 0;
	curctx = next_ctx;
	return;
}


