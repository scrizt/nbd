#ifndef NBD_STATEMACHINE_H
#define NBD_STATEMACHINE_H

#include <stdbool.h>

typedef struct nbd_state_ nbd_state;
typedef struct nbd_statemachine_ nbd_statemachine;
typedef struct nbd_statemachine_instance_ nbd_statemachine_instance;
typedef struct nbd_transition_ nbd_transition;

typedef void(*enter_state_t)(nbd_state* state, nbd_statemachine_instance* inst);

enum builtin_bits {
	/* per-export options */
	NBD_BIT_READONLY = 0,
	NBD_BIT_MULTIFILE,
	NBD_BIT_COW,
	NBD_BIT_AUTORO,
	NBD_BIT_SPARSE,
	NBD_BIT_SDP,
	NBD_BIT_SYNC,
	NBD_BIT_FLUSH,
	NBD_BIT_FUA,
	NBD_BIT_ROTATE,
	NBD_BIT_TRIM,
	NBD_BIT_FIXED,
	/* variable builtin states */
	NBD_BIT_READ_READY,
	NBD_BIT_WRITE_READY,
	NBD_BIT_COW_RESOLVED,
	NBD_BIT_MULTIFILE_RESOLVED,
	/* end of builtin marker */
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
void nbd_statemachine_set_bit(nbd_statemachine_instance*, int bit);
void nbd_statemachine_set_bits(nbd_statemachine_instance*, int bitcount, ...);
void nbd_statemachine_drop_bit(nbd_statemachine_instance*, int bit);
void nbd_statemachine_drop_bits(nbd_statemachine_instance*, int bitcount, ...);
void nbd_statemachine_check_bits(nbd_statemachine_instance*);

nbd_state* nbd_statemachine_add_state();
bool nbd_statemachine_set_init_state(nbd_statemachine*, nbd_state*);

nbd_transition* nbd_statemachine_add_transition(nbd_statemachine*, nbd_state* from, nbd_state* to);

bool nbd_transition_require_bit(nbd_transition*, int bit);
bool nbd_transition_forbid_bit(nbd_transition, int bit);

#endif
