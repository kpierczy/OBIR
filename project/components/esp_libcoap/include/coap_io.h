/* ============================================================================================================
 *  File:
 *  Author: Olaf Bergmann
 *  Source: https://github.com/obgm/libcoap/tree/develop/include/coap2
 *  Modified by: Krzysztof Pierczyk
 *  Modified time: 2020-11-20 13:46:52
 *  Description:
 * 
 *       File contains basic IO interface declaration for the library.
 * 
 *  Credits: 
 *
 *      This file is a modification of the original libcoap source file. Aim of the modification was to 
 *      provide cleaner, richer documented and ESP8266-optimised version of the library. Core API of the 
 *      project was not changed or expanded, although some elemenets (e.g. DTLS support) have been removed 
 *      due to lack of needings from the modifications' authors. 
 * 
 * ============================================================================================================ */


/* -------------------------------------------- [Original header] --------------------------------------------- */

/*
 * coap_io.h -- Default network I/O functions for libcoap
 *
 * Copyright (C) 2012-2013 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see README for terms
 * of use.
 */

/* ------------------------------------------------------------------------------------------------------------ */


#ifndef COAP_IO_H_
#define COAP_IO_H_

#include <assert.h>
#include <sys/types.h>
#include "address.h"

struct coap_packet_t;
struct coap_session_t;
struct coap_pdu_t;

struct coap_endpoint_t *coap_malloc_endpoint( void );
void coap_mfree_endpoint( struct coap_endpoint_t *ep );

/* ------------------------------------------- [Macrodefinitions] --------------------------------------------- */

/**
 * @brief: Size of the buffer caching incoming PDUs
 */
#ifndef COAP_RXBUFFER_SIZE
#define COAP_RXBUFFER_SIZE 1472
#endif

/**
 * @brief: States assoctaed with CoAP sockets' file descriptors
 */
#define coap_closesocket close
#define COAP_SOCKET_ERROR (-1)
#define COAP_INVALID_SOCKET (-1)

/**
 * @brief: Flags associated with @t coap_socket_flags_t type
 */
#define COAP_SOCKET_EMPTY        0x0000  /**< the socket is not used */
#define COAP_SOCKET_NOT_EMPTY    0x0001  /**< the socket is not empty */
#define COAP_SOCKET_BOUND        0x0002  /**< the socket is bound */
#define COAP_SOCKET_CONNECTED    0x0004  /**< the socket is connected */
#define COAP_SOCKET_WANT_READ    0x0010  /**< non blocking socket is waiting for reading */
#define COAP_SOCKET_WANT_WRITE   0x0020  /**< non blocking socket is waiting for writing */
#define COAP_SOCKET_WANT_ACCEPT  0x0040  /**< non blocking server socket is waiting for accept */
#define COAP_SOCKET_WANT_CONNECT 0x0080  /**< non blocking client socket is waiting for connect */
#define COAP_SOCKET_CAN_READ     0x0100  /**< non blocking socket can now read without blocking */
#define COAP_SOCKET_CAN_WRITE    0x0200  /**< non blocking socket can now write without blocking */
#define COAP_SOCKET_CAN_ACCEPT   0x0400  /**< non blocking server socket can now accept without blocking */
#define COAP_SOCKET_CAN_CONNECT  0x0800  /**< non blocking client socket can now connect without blocking */
#define COAP_SOCKET_MULTICAST    0x1000  /**< socket is used for multicast communication */

/**
 * @brief: Macro implemented for future usage
 */
#ifndef coap_mcast_interface
# define coap_mcast_interface(Local) 0
#endif

/* -------------------------------------------- [Data structures] --------------------------------------------- */

/**
 * @brief: CoAP-specific equivelent of the platform's file descriptor;
 *    file descriptors are used for net sockets' manipulation.
 */
typedef int coap_fd_t;

/**
 * @brief: Type of bit-flags associated with a socket
 */
typedef uint16_t coap_socket_flags_t;

/**
 * @brief: CoAP-specific socket's representatio
 */
typedef struct coap_socket_t {
  
    // Equivalent of the POSIX file descriptor
    coap_fd_t fd;
    // Socket's state (bit mask)
    coap_socket_flags_t flags;

} coap_socket_t;

/**
 * @brief: Represenation of the generic CoAP packet
 */
typedef struct coap_packet_t {

    // Source address of the packet
    coap_address_t src;
    // Destination address of the packet
    coap_address_t dst;
    // Index of the net interface
    int ifindex;

    // Buffer storing packet's payload
    unsigned char payload[COAP_RXBUFFER_SIZE];
    // Current length of the buffer
    size_t length;

} coap_packet_t;

/**
 * @brief: Types of NACK response reasons
 */
typedef enum {
    COAP_NACK_TOO_MANY_RETRIES,
    COAP_NACK_NOT_DELIVERABLE,
    COAP_NACK_RST,
} coap_nack_reason_t;


/* ----------------------------------------------- [Functions] ------------------------------------------------ */

/**
 * @brief: Creates and configures system UDP socket and associates it with @p sock structure.
 *    Connects created socket with @p server net address. If @p local_if address is given,
 *    socket is ALSO bound with this address for later listening.
 * 
 *    @p local_addr and @p remote_addr are actual addresses of the local socket assigned
 *    by the system and of remote server application conneted to.
 * 
 * @param sock:
 *    CoAP-specific to be configured
 * @param local_if:
 *    address to bind() with the @p sock
 * @param server:
 *    address of the server that socket will be connected with
 * @param default_port:
 *    default destination port set for the server's address if @p server's current
 *    port is set to 0
 * @param local_addr [out]:
 *    actual socket (address + port) assigned to the @p sock
 * @param remote_addr [out]:
 *    actual destination socket (address + port) that the @p socket has been connected with
 * @returns:
 *    0 if procedure fails, 1 otherwise
 */
int
coap_socket_connect_udp(coap_socket_t *sock,
                        const coap_address_t *local_if,
                        const coap_address_t *server,
                        int default_port,
                        coap_address_t *local_addr,
                        coap_address_t *remote_addr);

/**
 * @brief: Creates and configures system UDP socket and associates it with @p sock structure.
 *    Binds the socket with @p listen_addr. Actual address assigned by the system is copied
 *    to the @p bound_addr.
 * 
 * @param sock:
 *     CoAP-specific to be configured
 * @param listen_addr:
 *     address that socket will be connected with
 * @param bound_addr [out]:
 *     actual address assigned to the socket by the system
 * @returns:
 *    0 if procedure fails, 1 otherwise
 */
int
coap_socket_bind_udp(coap_socket_t *sock,
                     const coap_address_t *listen_addr,
                     coap_address_t *bound_addr );

/**
 * @brief: Creates and configures system TCP socket and associates it with @p sock structure.
 *    Connects created socket with @p server net address. If @p local_if address is given,
 *    socket is ALSO bound with this address for later listening.
 * 
 *    @p local_addr and @p remote_addr are actual addresses of the local socket assigned
 *    by the system and of remote server application conneted to.
 * 
 * @param sock:
 *    CoAP-specific to be configured
 * @param local_if:
 *    address to bind() with the @p sock
 * @param server:
 *    address of the server that socket will be connected with
 * @param default_port:
 *    default destination port set for the server's address if @p server's current
 *    port is set to 0
 * @param local_addr [out]:
 *    actual socket (address + port) assigned to the @p sock
 * @param remote_addr [out]:
 *    actual destination socket (address + port) that the @p socket has been connected with
 * @returns:
 *    0 if procedure fails, 1 otherwise
 */
int
coap_socket_connect_tcp1(coap_socket_t *sock,
                         const coap_address_t *local_if,
                         const coap_address_t *server,
                         int default_port,
                         coap_address_t *local_addr,
                         coap_address_t *remote_addr);


/**
 * @brief: Checks whether SO_ERROR flag of the system socket associated with @p sock is cleared.
 *    If so, function returns local address that socket is bound to and remote address the socket
 *    is connected with. Otherwise, closes the socket.  
 * @param sock:
 *    CoAP-specific to be configured
 * @param local_addr [out]:
 *    actual socket (address + port) assigned to the @p sock
 * @param remote_addr [out]:
 *    actual destination socket (address + port) that the @p socket has been connected with
 * @returns:
 *    0 if socket has been close, 1 otherwise
 */
int
coap_socket_connect_tcp2(coap_socket_t *sock,
                         coap_address_t *local_addr,
                         coap_address_t *remote_addr);

/**
 * @brief: Creates and configures system TCP socket and associates it with @p sock structure.
 *    Binds the socket with @p listen_addr. Actual address assigned by the system is copied
 *    to the @p bound_addr.
 * 
 * @param sock:
 *     CoAP-specific to be configured
 * @param listen_addr:
 *     address that socket will be connected with
 * @param bound_addr [out]:
 *     actual address assigned to the socket by the system
 * @returns:
 *    0 if procedure fails, 1 otherwise
 */
int
coap_socket_bind_tcp(coap_socket_t *sock,
                     const coap_address_t *listen_addr,
                     coap_address_t *bound_addr);

/**
 * @brief: Accepts connection incoming to the TCP socket associated with @p server. If acceptance
 *    succed, system socket dedicated for the new client is created and associated with @p new_client.
 *    Address of the newly opened socket is stored in the @p local_addr and the address of the new
 *    client is stored in the @p remote_addr.
 * 
 * @param server:
 *    socket desired to perform accept
 * @param new_client [out]:
 *    socket structure that will be associated with client-dedicated socket
 * @param local_addr [out]:
 *    address of the newly created socket
 * @param remote_addr [out]:
 *    address of the new client
 * @returns:
 *    0 if acceptance fails, 1 otherwise
 */
int
coap_socket_accept_tcp(coap_socket_t *server,
                       coap_socket_t *new_client,
                       coap_address_t *local_addr,
                       coap_address_t *remote_addr);

/**
 * @brief: Closes system socket associated with @p sock. Does nothing if @p sock is not associated
 *    with any system socket.
 * 
 * @param sock:
 *     socket associated with system socket ot be closed
 */
void coap_socket_close(coap_socket_t *sock);


/**
 * @brief: Sends datato the remote address associated with @p session. Uses system socket associated
 *    with @p sock as an interface.
 * 
 *    Internally uses network_send() method associated with the context that @p session is assigned to.
 * 
 * @param sock:
 *    interface socket to send data with
 * @param session;
 *    session associated with a destination address
 * @param data:
 *    data buffer to be sent
 * @param data_len:
 *    length of the @p data
 * @return ssize_t:
 *    number of bytes that have been sent, value lesser than 0 on error
 */
ssize_t
coap_socket_send( coap_socket_t *sock, struct coap_session_t *session,
                  const uint8_t *data, size_t data_len );

/**
 * @brief: Sends packet to the system socket associated with @p sock. The system socket,
 *    even UDP one, needs to be 'connected' as function uses internally send(), no sendto().
 * 
 * @param sock:
 *    socket associated with system socket to write to
 * @param data:
 *    buffer with outcoming data
 * @param data_len:
 *    size of the buffer
 * @return ssize_t:
 *    number of bytes sent, value lesser than 0 on error
 */
ssize_t
coap_socket_write(coap_socket_t *sock, const uint8_t *data, size_t data_len);


/**
 * @brief: Recieves packet from the system socket associated with @p sock. The system socket,
 *    even UDP one, needs to be 'connected' as function uses internally recv(), no recvfrom().
 * 
 * @param sock:
 *    socket associated with system socket to read from
 * @param data [out]:
 *    buffer for incoming data
 * @param data_len:
 *    size of the buffer
 * @return ssize_t:
 *    number of received bytes to the buffer, value lesser than 0 on error
 */
ssize_t
coap_socket_read(coap_socket_t *sock, uint8_t *data, size_t data_len);

/**
 * @brief: Main interface for data transmission. Sends data to the remote address associated 
 *    with @p session. Uses system socket associated with @p sock as an interface.
 *
 * @param sock:
 *    interface socket to send data with
 * @param session;
 *    session associated with a destination address
 * @param data:
 *    data buffer to be sent
 * @param data_len:
 *    length of the @p data
 * @return ssize_t:
 *    number of bytes that have been sent, value lesser than 0 on error
 */
ssize_t coap_network_send( coap_socket_t *sock, const struct coap_session_t *session, const uint8_t *data, size_t datalen );

/**
 * @rief: Main interface reading data. Reads data from the system socket associated with @p sock
 *    and puts it into @p packet. If system socket is not connected, data is read from 
 *    @p packet->src address. 
 *
 * @param sock:
 *    socket to read data from
 * @param packet:
 *    Received packet metadata and payload. src and dst should be preset.
 * @returns:
 *    the number of bytes received on success, value less than zero on error.
 */
ssize_t coap_network_read( coap_socket_t *sock, struct coap_packet_t *packet );

/**
 * @returns: current @v errno value in the humman-readable form
 */
const char *coap_socket_strerror( void );

/**
 * @brief: Given a packet, set msg and msg_len to an address and length of the packet's
 *    data in memory.
 * 
 * @param packet:
 *    packet to inspect
 * @param address [out]:
 *    pointer to the pointer do the packet's payload
 * @param length [out]:
 *    pointer to the packet's length
 */
void coap_packet_get_memmapped(struct coap_packet_t *packet,
                               unsigned char **address,
                               size_t *length);

/**
 * @brief: Sets packet's src and dst addresses to values given with @p src and @p dst.
 * 
 * @param packet:
 *    packet to be updated
 * @param src:
 *    desired source address of the packet
 * @param dst:
 *    desired destination address of the packet
 */
void coap_packet_set_addr( struct coap_packet_t *packet, const coap_address_t *src,
                           const coap_address_t *dst );

#endif /* COAP_IO_H_ */
