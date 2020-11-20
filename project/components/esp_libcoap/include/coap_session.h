/* ============================================================================================================
 *  File:
 *  Author: Olaf Bergmann
 *  Source: https://github.com/obgm/libcoap/tree/develop/include/coap2
 *  Modified by: Krzysztof Pierczyk
 *  Modified time: 2020-11-20 15:08:01
 *  Description:
 * 
 *      File contains header associated with CoAP session abstraction.
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

/* coap_session.h -- Session management for libcoap
*
* Copyright (C) 2017 Jean-Claue Michelou <jcm@spinetix.com>
*
* This file is part of the CoAP library libcoap. Please see
* README for terms of use.
*/

/* ------------------------------------------------------------------------------------------------------------ */


#ifndef COAP_SESSION_H_
#define COAP_SESSION_H_


#include "coap_io.h"
#include "coap_time.h"
#include "pdu.h"

struct coap_endpoint_t;
struct coap_context_t;
struct coap_queue_t;
struct coap_dtls_pki_t;

typedef struct coap_fixed_point_t coap_fixed_point_t;


/* ------------------------------------------- [Macrodefinitions] --------------------------------------------- */

/**
 * @brief: Basic parameters of the CoAP session
 */
#define COAP_DEFAULT_SESSION_TIMEOUT 300
#define COAP_PARTIAL_SESSION_TIMEOUT_TICKS (30 * COAP_TICKS_PER_SECOND)
#define COAP_DEFAULT_MAX_HANDSHAKE_SESSIONS 100

/**
 * @brief: Macro used to differentiate protocol type between TCP- and UDP-based
 */
#define COAP_PROTO_NOT_RELIABLE(p) ((p)==COAP_PROTO_UDP || (p)==COAP_PROTO_DTLS)
#define COAP_PROTO_RELIABLE(p) ((p)==COAP_PROTO_TCP || (p)==COAP_PROTO_TLS)


/**
 * @brief: possible values of @t coap_session_type_t type
 */
#define COAP_SESSION_TYPE_CLIENT 1  // Client-side session
#define COAP_SESSION_TYPE_SERVER 2  // Server-side session
#define COAP_SESSION_TYPE_HELLO  3  // Server-side ephemeral session for responding to a client's hello


/**
 * @brief: possible values of @t coap_session_state_t type
 */
#define COAP_SESSION_STATE_NONE        0
#define COAP_SESSION_STATE_CONNECTING  1
#define COAP_SESSION_STATE_HANDSHAKE   2
#define COAP_SESSION_STATE_CSM         3
#define COAP_SESSION_STATE_ESTABLISHED 4

/**
 * @brief: Number of seconds when to expect an ACK or a response to an outstanding
 *    CON message RFC 7252, Section 4.8 Default value of ACK_TIMEOUT is 2.
 */
#define COAP_DEFAULT_ACK_TIMEOUT ((coap_fixed_point_t){2,0})

/**
* @brief:A factor that is used to randomize the wait time before a message is retransmitted
     to prevent synchronization effects. RFC 7252, Section 4.8 Default value of ACK_RANDOM_FACTOR 
     is 1.5
*/
#define COAP_DEFAULT_ACK_RANDOM_FACTOR ((coap_fixed_point_t){1,500})

/**
 * @brief: Number of message retransmissions before message sending is stopped RFC 7252, Section
 *    4.8 Default value of MAX_RETRANSMIT is 4
 */
#define COAP_DEFAULT_MAX_RETRANSMIT  4

/**
 * @brief: The number of simultaneous outstanding interactions that a client maintains to a given
 *    server RFC 7252, Section 4.8 Default value of NSTART is 1
 */
#define COAP_DEFAULT_NSTART 1

/* -------------------------------------------- [Data structures] --------------------------------------------- */

/**
 * @brief: Abstraction of a fixed point number that can be used where necessary instead
 *    of a float. 1'000 fractional bits equals one integer
 */
struct coap_fixed_point_t {

    // Integer part
    uint16_t integer_part;
    // Fractional part (1/1000 precision)
    uint16_t fractional_part;
  
};

/**
 * @brief: Type of the CoAP session
 */
typedef uint8_t coap_session_type_t;

/**
 * @brief: State of the CoAP session (type of the realtionship with current peer)
 */
typedef uint8_t coap_session_state_t;


/**
 * @brief: Structure describing abstraction of the CoAP-level client-server session
 */
typedef struct coap_session_t {
  
    // Value used for storing sessions as a forward list
    struct coap_session_t *next;

    // Session's context
    struct coap_context_t *context;

    // Application-specific data
    void *app;                        

    /* ------------------------ Basic session info ------------------------------- */

    // Protocol used
    coap_proto_t proto;
    // Session's type ( @see @t coap_session_type_t)
    coap_session_type_t type;
    // Session's state (@see coap_session_state_t)
    coap_session_state_t state;
    // Count of refferences to the session from queues [?]
    unsigned ref;

    /* ----------------------- Session's parameters ------------------------------ */

    // Current MTU (i.e. maximum transimssion unit)
    unsigned mtu;
    // Maximum re-transmit count (default 4)
    unsigned int max_retransmit;      
    // Timeout for waiting for ack (default 2 secs)
    coap_fixed_point_t ack_timeout;   
    // ACK random factor backoff (default 1.5)
    coap_fixed_point_t ack_random_factor;

    /* ------------------------- Endpoints' info --------------------------------- */

    // Interface index [?]
    int ifindex;
    // Local interface address (optional) [?]
    coap_address_t local_if;
    // Remote address and port
    coap_address_t remote_addr;
    // Local address and port
    coap_address_t local_addr;
    // Socket object for the session (if any)
    coap_socket_t sock;               
    // Session's endpoint [?]
    struct coap_endpoint_t *endpoint; 

    /* -------------------------- Messages' info --------------------------------- */

    // The last message id that was used in this session
    uint16_t tx_mid;
    // Active CON request sent [?][mid/tag ?]
    uint8_t con_active;

    // List of delayed messages waiting to be sent
    struct coap_queue_t *delayqueue;

    // Number of bytes already written from the pdu at the head of sendqueue (if > 0)
    size_t partial_write;
    // Buffpr for header of incoming message header
    uint8_t read_header[8];
    // Number of bytes already read for an incoming message (if > 0)
    size_t partial_read;

    // Informations about incomplete incoming pdu [?]
    coap_pdu_t *partial_pdu;
    coap_tick_t last_rx_tx;
    coap_tick_t last_tx_rst;
    coap_tick_t last_ping;
    coap_tick_t last_pong;
    coap_tick_t csm_tx;
    
    uint8_t *psk_identity;
    size_t psk_identity_len;
    uint8_t *psk_key;
    size_t psk_key_len;

    /* ---------------------------- (D)TLS info ---------------------------------- */

    // Overhead of TLS layer [?]
    unsigned tls_overhead;
    // Security parameters [?]
    void *tls;
    // (D)TLS events tracked on this sesison
    int dtls_event;
    // DTLS setup retry counter
    unsigned int dtls_timeout_count;
    
} coap_session_t;

/**
* @brief: Abstraction of a virtual endpoint that can be attached to @t coap_context_t. The
*    tuple (handle, addr) must uniquely identify this endpoint.
*/
typedef struct coap_endpoint_t {

    // Value used for storing sessions as a forward list
    struct coap_endpoint_t *next;

    // Endpoint's context
    struct coap_context_t *context; 

    // Protocol used on this interface
    coap_proto_t proto;

    // Default mtu for this interface
    uint16_t default_mtu;
    
    // Socket object for the interface (if any)
    coap_socket_t sock;
    // Local interface address
    coap_address_t bind_addr;

    // list of active sessions
    coap_session_t *sessions;
    // Distinguished session of DTLS hello messages
    coap_session_t hello;

} coap_endpoint_t;


/* ----------------------------------------------- [Functions] ------------------------------------------------ */

/**
 * @brief: Increment reference counter on a session.
 *
 * @param:
 *    session The CoAP session.
 * @returns: 
 *    same as @p session
 */
coap_session_t *coap_session_reference(coap_session_t *session);

/**
 * @brief: Decrement reference counter on a session
 *
 * @param session:
 *    the CoAP session.
 * 
 * @note: the session may be deleted as a result and should not be used after this call.
 */
void coap_session_release(coap_session_t *session);

/**
 * @brief: Stores data within the given session. This function overwrites any value
 *    that has previously been stored with @p session->app.
 * 
 * @param session:
 *    session to be modified
 * @param data:
 *    data to be assigned to the @p session
 */
void coap_session_set_app_data(coap_session_t *session, void *data);

/**
 * @param session:
 *    session to be inspected
 * @returns:
 *    any application-specific data that has been stored with @p session->app using the 
 *    function @f coap_session_set_app_data(). This function will return NULL if no data
 *    has been stored.
 */
void *coap_session_get_app_data(const coap_session_t *session);

/**
 * @brief: Notify session that it has failed.
 *
 * @param session:
 *    the CoAP session
 * @param reason:
 *    the reason why the session was disconnected
 */
void coap_session_disconnected(coap_session_t *session, coap_nack_reason_t reason);

/**
 * @brief: Notify session transport has just connected and CSM (Capabilities and Settings Message)
 *    exchange can now start.
 *
 * @param session:
 *    the CoAP session.
 */
void coap_session_send_csm(coap_session_t *session);

/**
 * @brief: Notifies session that it has just connected or reconnected.
 *
 * @param session:
 *    the CoAP session.
 */
void coap_session_connected(coap_session_t *session);

/**
 * @brief: Sets the session MTU. This is the maximum message size that can be sent,
 *    excluding IP and UDP overhead.
 *
 * @param session:
 *    the CoAP session.
 * @param mtu:
 *    maximum message size
 */
void coap_session_set_mtu(coap_session_t *session, unsigned mtu);

/**
 * @brief: Get maximum acceptable PDU size
 *
 * @param session:
 *    the CoAP session.
 * @return maximum:
 *    PDU size, not including header (but including token).
 */
size_t coap_session_max_pdu_size(const coap_session_t *session);

/**
 * @brief: Creates a new client session to the designated server.
 *
 * @param ctx:
 *    the CoAP context.
 * @param local_if:
 *    address of local interface. It is recommended to use NULL to let the operating 
 *    system choose a suitable local interface. If an address is specified, the port
 *    number should be zero, which means that a free port is automatically selected.
 * 
 * @param server:
 *    the server's address. If the port number is zero, the default port for the
 *    protocol will be used.
 * @param proto:
 *    protocol type
 * @returns:
 *    a new CoAP session or NULL if failed. Call coap_session_release to free.
 */
coap_session_t *coap_new_client_session(
  struct coap_context_t *ctx,
  const coap_address_t *local_if,
  const coap_address_t *server,
  coap_proto_t proto
);

/**
 * @brief: Creates a new client session to the designated server with PSK credentials
 * @param ctx:
 *    the CoAP context.
 * @param local_if:
 *    address of local interface. It is recommended to use NULL to let the operating
 *    system choose a suitable local  interface. If an address is specified, the port
 *    number should be zero, which means that a free port is automatically selected.
 * @param server:
 *    the server's address. If the port number is zero, the default port for the protocol 
 *    will be used.
 * @param proto:
 *    Protocol.
 * @param identity:
 *    PSK client identity
 * @param key:
 *    PSK shared key
 * @param key_len:
 *    PSK shared key length
 *
 * @return A new CoAP session or NULL if failed. Call coap_session_release to free.
 */
coap_session_t *coap_new_client_session_psk(
  struct coap_context_t *ctx,
  const coap_address_t *local_if,
  const coap_address_t *server,
  coap_proto_t proto,
  const char *identity,
  const uint8_t *key,
  unsigned key_len
);

/**
 * @brief: Creates a new client session to the designated server with PKI credentials
 *
 * @param ctx:
 *    the CoAP context.
 * @param local_if:
 *    atddress of local interface. It is recommended to use NULL to let the operating system
 *    choose a suitable local interface. If an address is specified, the port number should 
 *    be zero, which means that a free port is automatically selected.
 * @param server:
 *    the server's address. If the port number is zero, the default port for the protocol 
 *    will be used.
 * @param proto:
 *    CoAP Protocol.
 * @param setup_data:
 *    PKI parameters.
 * @returns:
 *    a new CoAP session or NULL if failed. Call coap_session_release() to free.
 */
coap_session_t *coap_new_client_session_pki(
  struct coap_context_t *ctx,
  const coap_address_t *local_if,
  const coap_address_t *server,
  coap_proto_t proto,
  struct coap_dtls_pki_t *setup_data
);

/**
 * @brief: Creates a new server session for the specified endpoint.
 * 
 * @param ctx:
 *    the CoAP context.
 * @param ep:
 *    an endpoint where an incoming connection request is pending.
 *
 * @returns:
 *    a new CoAP session or NULL if failed. Call coap_session_release to free.
 */
coap_session_t *coap_new_server_session(
  struct coap_context_t *ctx,
  struct coap_endpoint_t *ep
);

/**
 * @brief: Function interface for datagram data transmission. This function returns the
 *    number of bytes that have been transmitted, or a value less than zero on error.
 *
 * @param session:
 *    session to send data on.
 * @param data:
 *    the data to send.
 * @param datalen:
 *    the actual length of @p data.
 * @returns:
 *    the number of bytes written on success, or a value less than zero on error.
 */
ssize_t coap_session_send(coap_session_t *session,
  const uint8_t *data, size_t datalen);

/**
 * @brief: Function interface for stream data transmission. This function returns the number
 *    of bytes that have been transmitted, or a value less than zero on error. The number of
 *    bytes written may be less than datalen because of congestion control.
 *
 * @param session:
 *    session to send data on.
 * @param data:
 *    the data to send.
 * @param datalen:
 *    the actual length of @p data.
 * @returns:
 *    the number of bytes written on success, or a value less than zero on error.
 */
ssize_t coap_session_write(coap_session_t *session,
  const uint8_t *data, size_t datalen);

/**
 * @brief: Send a pdu according to the session's protocol. This function returns the number of
 *    bytes that have been transmitted, or a value less than zero on error.
 * 
 * @param session:
 *    session to send pdu on.
 * @param pdu:
 *    the pdu to send.
 *
 * @returns:
 *    the number of bytes written on success, or a value less than zero on error.
 */
ssize_t coap_session_send_pdu(coap_session_t *session, coap_pdu_t *pdu);

/**
 * @brief: Get session description.
 *
 * @param session:
 *   the CoAP session.
 * @returns:
 *   description string.
 */
const char *coap_session_str(const coap_session_t *session);

/**
 * @brief 
 * 
 * @param session 
 * @param pdu 
 * @param node 
 * @return ssize_t 
 */
ssize_t
coap_session_delay_pdu(coap_session_t *session, coap_pdu_t *pdu,
                       struct coap_queue_t *node);

/**
* 
*
* @param context        
* @param listen_addr: 
* @param proto          Protocol used on this endpoint
*/

/**
 * @brief: Create a new endpoint for communicating with peers.
 * 
 * @param context:
 *    the coap context that will own the new endpoint
 * @param listen_addr:
 *    address the endpoint will listen for incoming requests on or originate outgoing
 *    requests from. Use NULL to specify that no incoming request will be accepted and 
 *    use a random endpoint.
 * @param proto:
 *     
 * @returns:
 *
 */
coap_endpoint_t *coap_new_endpoint(struct coap_context_t *context, const coap_address_t *listen_addr, coap_proto_t proto);

/**
 * @brief: Set the endpoint's default MTU. This is the maximum message size that can be
 *    sent, excluding IP and UDP overhead.
 *
 * @param endpoint:
 *    the CoAP endpoint.
 * @param mtu:
 *    maximum message size
 */
void coap_endpoint_set_default_mtu(coap_endpoint_t *endpoint, unsigned mtu);

/**
 * @brief: 
 * 
 * @param ep:
 *     
 */
void coap_free_endpoint(coap_endpoint_t *ep);


/**
 * @brief: Get endpoint description.
 *
 * @param endpoint:
 *    the CoAP endpoint.
 * @returns:
 *    description string.
 */
const char *coap_endpoint_str(const coap_endpoint_t *endpoint);

/**
 * @brief: Lookup the server session for the packet received on an endpoint, or create
 *    a new one.
 *
 * @param endpoint:
 *    active endpoint the packet was received on.
 * @param packet:
 *    received packet.
 * @param now:
 *    the current time in ticks.
 * @returns:
 *    the CoAP session.
 */
coap_session_t *coap_endpoint_get_session(coap_endpoint_t *endpoint,
  const struct coap_packet_t *packet, coap_tick_t now);

/**
 * @brief: Create a new DTLS session for the @p endpoint.
 *
 * @param endpoint:
 *    endpoint to add DTLS session to
 * @param packet:
 *    received packet information to base session on.
 * @param now:
 *    the current time in ticks.
 *
 * @returns:
 *    created CoAP session or @c NULL if error.
 */
coap_session_t *coap_endpoint_new_dtls_session(coap_endpoint_t *endpoint,
  const struct coap_packet_t *packet, coap_tick_t now);

/**
 * @brief:
 * 
 * @param ctx:
 *    
 * @param remote_addr:
 *    
 * @param ifindex:
 *    
 * @returns:
 *    coap_session_t* 
 */
coap_session_t *coap_session_get_by_peer(struct coap_context_t *ctx,
  const struct coap_address_t *remote_addr, int ifindex);

/**
 * @brief:
 * 
 * @param session:
 *    
 */
void coap_session_free(coap_session_t *session);

/**
 * @brief:
 * 
 * @param session:
 *     
 */
void coap_session_mfree(coap_session_t *session);

/**
 * @brief: Set the CoAP maximum retransmit count before failure. Number of message
 *    retransmissions before message sending is stopped
 *
 * @param session:
 *    the CoAP session.
 * @param value:
 *    the value to set to. The default is 4 and should not normally get changed.
 */
void coap_session_set_max_retransmit(coap_session_t *session,
                                     unsigned int value);

/**
 * @brief: Set the CoAP initial ack response timeout before the next re-transmit. 
 *    Number of seconds when to expect an ACK or a response to an outstanding CON
 *    message.
 *
 * @param session:
 *    the CoAP session.
 * @param value:
 *    the value to set to. The default is 2 and should not normally get changed.
 */
void coap_session_set_ack_timeout(coap_session_t *session,
                                  coap_fixed_point_t value);

/**
 * @brief: Set the CoAP ack randomize factor. A factor that is used to randomize the
 *    wait time before a message is retransmitted to prevent synchronization effects.
 *
 * @param session:
 *    the CoAP session.
 * @param value:
 *    the value to set to. The default is 1.5 and should not normally get changed.
 */
void coap_session_set_ack_random_factor(coap_session_t *session,
                                        coap_fixed_point_t value);

/**
 * @brief: Get the CoAP maximum retransmit before failure. Number of message retransmissions 
 *    before message sending is stopped
 *
 * @param session:
 *    the CoAP session.
 * @returns:
 *    current maximum retransmit value
 */
unsigned int coap_session_get_max_transmit(coap_session_t *session);

/**
 * @brief: Get the CoAP initial ack response timeout before the next re-transmit. Number of
 *    seconds when to expect an ACK or a response to an outstanding CON message.
 *
 * @param session:
 *    the CoAP session.
 * @returns:
 *    current ack response timeout value
 */
coap_fixed_point_t coap_session_get_ack_timeout(coap_session_t *session);

/**
 * @brief: Get the CoAP ack randomize factor. A factor that is used to randomize the wait time
 *    before a message is retransmitted to prevent synchronization effects.
 *
 * @param session:
 *    the CoAP session.
 * @returns:
 *    current ack randomize value
 */
coap_fixed_point_t coap_session_get_ack_random_factor(coap_session_t *session);

/**
 * @brief: Send a ping message for the session.
 * 
 * @param session:
 *    the CoAP session.
 * @returns:
 *    COAP_INVALID_TID if there is an error
 */
coap_tid_t coap_session_send_ping(coap_session_t *session);

#endif  /* COAP_SESSION_H */
