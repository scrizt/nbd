#include <nbd/statemachine.h>
#include <glib.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>

struct nbd_statemachine_ {
	int max_bit;
	int instances;
	nbd_state* init_state;
};

struct nbd_sm_instance_priv {
	GArray* bits;
};

struct nbd_transition_ {
	nbd_state* to;
	GArray* bits_require;
	GArray* bits_forbid;
};

struct nbd_state_private {
	GArray* out;
};

nbd_statemachine* nbd_statemachine_new() {
	nbd_statemachine* retval = calloc(sizeof(nbd_statemachine*), 1);
	retval->max_bit = NBD_BIT_USER_START - 1;
}

nbd_statemachine_instance* nbd_statemachine_new_instance(nbd_statemachine* mach) {
	nbd_statemachine_instance* retval = calloc(sizeof(nbd_statemachine_instance), 1);
	mach->instances++;
	retval->priv = calloc(sizeof(struct nbd_sm_instance_priv), 1);
	retval->priv->bits = g_array_new(FALSE, TRUE, sizeof(uint64_t));
	g_array_set_size(retval->priv->bits, (mach->max_bit / 64) + 1);
	retval->curstate = mach->init_state;
}

void nbd_statemacine_set_bit(nbd_statemachine_instance* inst, int bit) {
	assert(bit <= inst->machine->max_bit);

	int bit_apos = bit / 64;
	int bit_bpos = bit % 64;
	g_array_index(inst->priv->bits, uint64_t, bit_apos) |= (uint64_t)1 << bit_bpos;
}

void nbd_statemachine_set_bits(nbd_statemachine_instance* inst, int bitcount, ...) {
	va_list ap;
	va_start(ap, bitcount);
	for(int i=0; i<bitcount; i++) {
		int bit = va_arg(ap, int);
		nbd_statemachine_set_bit(inst, bit);
	}
	va_end(ap);
}

void nbd_statemachine_drop_bit(nbd_statemachine_instance* inst, int bit) {
	assert(bit <= inst->machine->max_bit);

	int bit_apos = bit / 64;
	int bit_bpos = bit % 64;
	g_array_index(inst->priv->bits, uint64_t, bit_apos) &= ~((uint64_t)1 << bit_bpos);
}

void nbd_statemachine_drop_bits(nbd_statemachine_instance* inst, int bitcount, ...) {
	va_list ap;
	va_start(ap, bitcount);
	for(int i=0; i<bitcount; i++) {
		int bit = va_arg(ap, int);
		nbd_statemachine_drop_bit(inst, bit);
	}
	va_end(ap);
}

void nbd_statemachine_check_bits(nbd_statemachine_instance* inst) {
	for(int i=0; i<inst->curstate->priv->out->len; i++) {
		nbd_transition* t = g_array_index(inst->curstate->priv->out, nbd_transition*, i);
		for(int j=0; j<=inst->machine->max_bit / 64; j++) {
			if( ((g_array_index(inst->priv->bits, uint64_t, j) & g_array_index(t->bits_require, uint64_t, j)) == g_array_index(t->bits_require, uint64_t, j))
			 && ((g_array_index(inst->priv->bits, uint64_t, j) & g_array_index(t->bits_forbid,  uint64_t, j)) == 0)) {
				inst->curstate = t->to;
				inst->curstate->enter_state(inst->curstate, inst);
				return;
			}
		}
	}
}
