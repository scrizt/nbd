#ifndef NBD_STATEMACHINE_H
#define NBD_STATEMACHINE_H

#include <stdbool.h>

typedef struct nbd_state_ nbd_state;
typedef struct nbd_statemachine_ nbd_statemachine;
typedef struct nbd_statemachine_instance_ nbd_statemachine_instance;
typedef struct nbd_transition_ nbd_transition;

typedef void(*enter_state_t)(nbd_state* state, nbd_statemachine_instance* inst);

enum builtin_bits {
	NBD_BIT_READ_READY = 1,
	NBD_BIT_WRITE_READY,
	NBD_BIT_COW_RESOLVE,
	NBD_BIT_MULTIFILE_RESOLVE,
	NBD_BIT_USER_START,
};

struct nbd_state_ {
	struct nbd_state_private* priv;
	nbd_statemachine* machine;
	enter_state_t enter_state;
};

struct nbd_statemachine_instance_ {
	struct nbd_sm_instance_priv* priv;
	nbd_statemachine* machine;
	nbd_state* curstate;
	void* data;
};

nbd_statemachine* nbd_statemachine_new();
nbd_statemachine_instance* nbd_statemachine_new_instance(nbd_statemachine*);

int nbd_statemachine_add_bit(nbd_statemachine*);
bool nbd_statemachine_get_bit(nbd_statemachine_instance*, int bit);
bool nbd_statemachine_set_bit(nbd_statemachine_instance*, int bit);

nbd_state* nbd_statemachine_add_state();
bool nbd_statemachine_set_init_state(nbd_statemachine*, nbd_state*);

nbd_transition* nbd_statemachine_add_transition(nbd_statemachine*, nbd_state* from, nbd_state* to);

bool nbd_transition_require_bit(nbd_transition*, int bit);
bool nbd_transition_forbid_bit(nbd_transition, int bit);

#endif
