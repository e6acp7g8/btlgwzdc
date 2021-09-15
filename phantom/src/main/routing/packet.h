/*
 * The Shadow Simulator
 * Copyright (c) 2010-2011, 
 * See LICENSE for licensing information
 */

#ifndef SHD_PACKET_H_
#define SHD_PACKET_H_

#include <glib.h>
#include <netinet/in.h>

#include "main/routing/packet.minimal.h"

#include "main/core/support/definitions.h"
#include "main/host/protocol.h"
#include "main/host/syscall_types.h"
#include "main/host/thread.h"

typedef struct _PacketTCPHeader PacketTCPHeader;
struct _PacketTCPHeader {
    enum ProtocolTCPFlags flags;
    in_addr_t sourceIP;
    in_port_t sourcePort;
    in_addr_t destinationIP;
    in_port_t destinationPort;
    guint sequence;
    guint acknowledgment;
    GList* selectiveACKs;
    guint window;
    SimulationTime timestampValue;
    SimulationTime timestampEcho;
};

const gchar* protocol_toString(ProtocolType type);

Packet* packet_new(Host* host);
void packet_setPayload(Packet* packet, Thread* thread, PluginVirtualPtr payload,
                       gsize payloadLength);
Packet* packet_copy(Packet* packet);

void packet_ref(Packet* packet);
void packet_unref(Packet* packet);
static inline void packet_unrefTaskFreeFunc(gpointer packet) { packet_unref(packet); }

void packet_setPriority(Packet *packet, double value);

void packet_setLocal(Packet* packet, enum ProtocolLocalFlags flags,
        gint sourceDescriptorHandle, gint destinationDescriptorHandle, in_port_t port);
void packet_setUDP(Packet* packet, enum ProtocolUDPFlags flags,
        in_addr_t sourceIP, in_port_t sourcePort,
        in_addr_t destinationIP, in_port_t destinationPort);
void packet_setTCP(Packet* packet, enum ProtocolTCPFlags flags,
        in_addr_t sourceIP, in_port_t sourcePort,
        in_addr_t destinationIP, in_port_t destinationPort, guint sequence);

void packet_updateTCP(Packet* packet, guint acknowledgement, GList* selectiveACKs,
        guint window, SimulationTime timestampValue, SimulationTime timestampEcho);

guint packet_getPayloadLength(const Packet* packet);
gdouble packet_getPriority(const Packet* packet);
guint packet_getHeaderSize(Packet* packet);

in_addr_t packet_getDestinationIP(Packet* packet);
in_port_t packet_getDestinationPort(Packet* packet);
in_addr_t packet_getSourceIP(Packet* packet);
in_port_t packet_getSourcePort(Packet* packet);
ProtocolType packet_getProtocol(Packet* packet);

gssize packet_copyPayload(const Packet* packet, Thread* thread, gsize payloadOffset,
                          PluginVirtualPtr buffer, gsize bufferLength);
guint packet_copyPayloadShadow(Packet* packet, gsize payloadOffset, void* buffer,
                               gsize bufferLength);
GList* packet_copyTCPSelectiveACKs(Packet* packet);
PacketTCPHeader* packet_getTCPHeader(Packet* packet);
gint packet_compareTCPSequence(Packet* packet1, Packet* packet2, gpointer user_data);

void packet_addDeliveryStatus(Packet* packet, PacketDeliveryStatusFlags status);
PacketDeliveryStatusFlags packet_getDeliveryStatus(Packet* packet);

gchar* packet_toString(Packet* packet);

#endif /* SHD_PACKET_H_ */
