#include <nbd/statemachine.h>
#include <nbd/server-data.h>

void setup_instance(nbd_state*, nbd_statemachine_instance* inst) {
	SERVER* server = (SERVER*) inst->data;
	nbd_statemachine_int_set_bitmask(inst, server->flags);
	nbd_statemachine_set_bit(inst, NBD_STATE_INITED);
}
