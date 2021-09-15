/*
 * The Shadow Simulator
 * Copyright (c) 2010-2011, 
 * See LICENSE for licensing information
 */

#ifndef SHD_SOCKET_H_
#define SHD_SOCKET_H_

#include <glib.h>
#include <netinet/in.h>
#include <sys/un.h>

#include "main/core/support/definitions.h"
#include "main/host/descriptor/descriptor_types.h"
#include "main/host/descriptor/transport.h"
#include "main/host/protocol.h"
#include "main/routing/packet.minimal.h"

typedef struct _Socket Socket;
typedef struct _SocketFunctionTable SocketFunctionTable;

typedef gboolean (*SocketIsFamilySupportedFunc)(Socket* socket, sa_family_t family);
typedef gint (*SocketConnectToPeerFunc)(Socket* socket, Host* host, in_addr_t ip, in_port_t port,
                                        sa_family_t family);
typedef void (*SocketProcessFunc)(Socket* socket, Host* host, Packet* packet);
typedef void (*SocketDropFunc)(Socket* socket, Host* host, Packet* packet);

struct _SocketFunctionTable {
    DescriptorCloseFunc close;
    DescriptorFreeFunc free;
    TransportSendFunc send;
    TransportReceiveFunc receive;
    SocketProcessFunc process;
    SocketIsFamilySupportedFunc isFamilySupported;
    SocketConnectToPeerFunc connectToPeer;
    SocketDropFunc dropPacket;
    MAGIC_DECLARE_ALWAYS;
};

enum SocketFlags {
    SF_NONE = 0,
    SF_BOUND = 1 << 0,
    SF_UNIX = 1 << 1,
    SF_UNIX_BOUND = 1 << 2,
};

struct _Socket {
    Transport super;
    SocketFunctionTable* vtable;

    enum SocketFlags flags;
    ProtocolType protocol;

    in_addr_t peerIP;
    in_addr_t peerPort;
    gchar* peerString;

    in_addr_t boundAddress;
    in_port_t boundPort;
    gchar* boundString;

    gchar* unixPath;

    /* buffering packets readable by user */
    GQueue* inputBuffer;
    gsize inputBufferSize;
    gsize inputBufferSizePending;
    gsize inputBufferLength;

    /* buffering packets ready to send */
    GQueue* outputBuffer;
    GQueue* outputControlBuffer;
    gsize outputBufferSize;
    gsize outputBufferSizePending;
    gsize outputBufferLength;

    MAGIC_DECLARE_ALWAYS;
};

void socket_init(Socket* socket, Host* host, SocketFunctionTable* vtable, LegacyDescriptorType type,
                 guint receiveBufferSize, guint sendBufferSize);

void socket_pushInPacket(Socket* socket, Host* host, Packet* packet);
void socket_dropPacket(Socket* socket, Host* host, Packet* packet);
Packet* socket_pullOutPacket(Socket* socket, Host* host);
Packet* socket_peekNextOutPacket(const Socket* socket);
Packet* socket_peekNextInPacket(const Socket* socket);

gsize socket_getInputBufferSize(Socket* socket);
void socket_setInputBufferSize(Socket* socket, gsize newSize);
gsize socket_getInputBufferLength(Socket* socket);
gsize socket_getInputBufferSpace(Socket* socket);
gboolean socket_addToInputBuffer(Socket* socket, Host* host, Packet* packet);
Packet* socket_removeFromInputBuffer(Socket* socket, Host* host);

gsize socket_getOutputBufferSize(Socket* socket);
void socket_setOutputBufferSize(Socket* socket, gsize newSize);
gsize socket_getOutputBufferLength(Socket* socket);
gsize socket_getOutputBufferSpace(Socket* socket);
gboolean socket_addToOutputBuffer(Socket* socket, Host* host, Packet* packet);
Packet* socket_removeFromOutputBuffer(Socket* socket, Host* host);

gboolean socket_isBound(Socket* socket);
gboolean socket_getPeerName(Socket* socket, in_addr_t* ip, in_port_t* port);
void socket_setPeerName(Socket* socket, in_addr_t ip, in_port_t port);
gboolean socket_getSocketName(Socket* socket, in_addr_t* ip, in_port_t* port);
void socket_setSocketName(Socket* socket, in_addr_t ip, in_port_t port);

ProtocolType socket_getProtocol(Socket* socket);

gboolean socket_isFamilySupported(Socket* socket, sa_family_t family);
gint socket_connectToPeer(Socket* socket, Host* host, in_addr_t ip, in_port_t port,
                          sa_family_t family);

gboolean socket_isUnix(Socket* socket);
void socket_setUnix(Socket* socket, gboolean isUnixSocket);
void socket_setUnixPath(Socket* socket, const gchar* path, gboolean isBound);
gchar* socket_getUnixPath(Socket* socket);

#endif /* SHD_SOCKET_H_ */
