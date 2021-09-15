/*
 * The Shadow Simulator
 * Copyright (c) 2010-2011, 
 * See LICENSE for licensing information
 */

#ifndef SHD_CONTROLLER_H_
#define SHD_CONTROLLER_H_

#include <glib.h>

typedef struct _Controller Controller;

#include "main/bindings/c/bindings.h"
#include "main/core/support/definitions.h"
#include "main/routing/address.h"
#include "main/routing/dns.h"
#include "main/routing/topology.h"

Controller* controller_new(ConfigOptions*);
void controller_free(Controller*);
gint controller_run(Controller*);

void controller_updateMinTimeJump(Controller*, gdouble);
gdouble controller_getRunTimeElapsed(Controller*);

gboolean controller_managerFinishedCurrentRound(Controller*, SimulationTime, SimulationTime*,
                                                SimulationTime*);
gdouble controller_getLatency(Controller* controller, Address* srcAddress, Address* dstAddress);

// TODO remove these eventually since they cant be shared accross remote managers
DNS* controller_getDNS(Controller* controller);
Topology* controller_getTopology(Controller* controller);

#endif /* SHD_CONTROLLER_H_ */
