#ifndef LIBNBD_H
#define LIBNBD_H

/*
 * Network Block Device - library
 *
 * Copyright (c) 1996-1998 Pavel Machek
 * Copyright (c) 2002-2013 Wouter Verhelst
 * Copyright (c) 2002 Anton Altaparmakov
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License, version 2, as
 *  published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301 USA.
 */

#include <glib.h>

typedef struct nbds_exp_ NBDS_EXPORT;

/*
 * state machine handling
 */
typedef struct nbds_state_ NBDS_STATE;

typedef void(*state_handler)(NBDS_STATE*, int socket, NBDS_EXPORT* exp);

enum {
	NBDS_STATE_OPEN,
	NBDS_STATE_NEGOTIATING,
	NBDS_STATE_REJECTED,
	NBDS_STATE_WAITING,
	NBDS_STATE_REQS,
	NBDS_STATE_FILE_RESOLV,
	NBDS_STATE_SPLIT_WAIT,
	NBDS_STATE_WRITE_READY,
	NBDS_STATE_FLUSH,
	NBDS_STATE_FIN,
	NBDS_STATE_USER = 100,
};

typedef struct {
	enum {
		NBDS_COND_PREVREADY,
		NBDS_COND_PREVNSKIP,
		NBDS_COND_PREVFAIL,
		NBDS_COND_SOCKREADY,
	} condtype;
	/* XXX */
} NBDS_COND;

NBDS_STATE* nbds_new_state();
int nbds_state_get_number(NBDS_STATE*);
NBDS_STATE* nbds_state_get_number(int);

void nbds_state_set_handler(NBDS_STATE*, state_handler);
void nbds_state_set_condition_type(NBDS_STATE*, 

#endif
