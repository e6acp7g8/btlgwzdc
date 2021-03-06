/*
 * The Shadow Simulator
 * Copyright (c) 2010-2011, 
 * See LICENSE for licensing information
 */

#ifndef SHD_NETWORK_INTERFACE_H_
#define SHD_NETWORK_INTERFACE_H_

#include <glib.h>
#include <netinet/in.h>

#include "main/core/support/options.h"
#include "main/host/descriptor/socket.h"
#include "main/host/protocol.h"
#include "main/routing/address.h"
#include "main/routing/router.h"

typedef struct _NetworkInterface NetworkInterface;

NetworkInterface* networkinterface_new(Address* address, guint64 bwDownKiBps, guint64 bwUpKiBps,
        gboolean logPcap, gchar* pcapDir, QDiscMode qdisc, guint64 interfaceReceiveLength);
void networkinterface_free(NetworkInterface* interface);

Address* networkinterface_getAddress(NetworkInterface* interface);
guint32 networkinterface_getSpeedUpKiBps(NetworkInterface* interface);
guint32 networkinterface_getSpeedDownKiBps(NetworkInterface* interface);

gboolean networkinterface_isAssociated(NetworkInterface* interface, ProtocolType type,
        in_port_t port, in_addr_t peerAddr, in_port_t peerPort);
void networkinterface_associate(NetworkInterface* interface, Socket* transport);
void networkinterface_disassociate(NetworkInterface* interface, Socket* transport);

void networkinterface_wantsSend(NetworkInterface* interface, Socket* transport);
void networkinterface_sent(NetworkInterface* interface);

void networkinterface_startRefillingTokenBuckets(NetworkInterface* interface);

void networkinterface_setRouter(NetworkInterface* interface, Router* router);
Router* networkinterface_getRouter(NetworkInterface* interface);

void networkinterface_receivePackets(NetworkInterface* interface);

#endif /* SHD_NETWORK_INTERFACE_H_ */
