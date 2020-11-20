/* ============================================================================================================
 *  File:
 *  Author: Olaf Bergmann
 *  Source: https://github.com/obgm/libcoap/tree/develop/include/coap2
 *  Modified by: Krzysztof Pierczyk
 *  Modified time: 2020-11-19 20:11:10
 *  Description:
 * 
 *      File contains API related to inta-library, library-specific events handling.
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
 * coap_event.h -- libcoap Event API
 *
 * Copyright (C) 2016 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see README for terms
 * of use.
 */

/* ------------------------------------------------------------------------------------------------------------ */


#ifndef COAP_EVENT_H_
#define COAP_EVENT_H_

#include "libcoap.h"

struct coap_context_t;
struct coap_session_t;


/* ------------------------------------------- [Macrodefinitions] --------------------------------------------- */

/**
 * @brief: (D)TLS events for COAP_PROTO_DTLS and COAP_PROTO_TLS
 */
#define COAP_EVENT_DTLS_CLOSED        0x0000
#define COAP_EVENT_DTLS_CONNECTED     0x01DE
#define COAP_EVENT_DTLS_RENEGOTIATE   0x01DF
#define COAP_EVENT_DTLS_ERROR         0x0200

/**
 * @brief: TCP events for COAP_PROTO_TCP and COAP_PROTO_TLS
 */
#define COAP_EVENT_TCP_CONNECTED      0x1001
#define COAP_EVENT_TCP_CLOSED         0x1002
#define COAP_EVENT_TCP_FAILED         0x1003

/**
 * @brief: CSM exchange events for reliable protocols only
 */
#define COAP_EVENT_SESSION_CONNECTED  0x2001
#define COAP_EVENT_SESSION_CLOSED     0x2002
#define COAP_EVENT_SESSION_FAILED     0x2003


/* -------------------------------------------- [Data structures] --------------------------------------------- */

/**
 * @brief: Scalar type to representing different events, e.g. DTLS events or
 *    retransmission timeouts.
 */
 typedef unsigned int coap_event_t;

/**
 * @brief: Type for event handler functions that can be registered with a CoAP context
 *    using the @f coap_set_event_handler() function. The handler will be called by
 *    the library at after event's occuring.
 * 
 * @param context:
 *     libcoap context that registered handler will be associated with
 * @param event:
 *     event that triggered the handler
 * @param session:
 *     session during which the event occured
 *     
 */
typedef int (*coap_event_handler_t)(struct coap_context_t *context,
                                    coap_event_t event,
                                    struct coap_session_t *session);


/* ----------------------------------------- [Deprecated functions] ------------------------------------------- */

/**
 * @brief: Registers the function @p hnd as callback for events from the given
 *    CoAP context @p context. Any event handler that has previously been
 *    registered with @p context will be overwritten by this operation.
 *
 * @param context:
 *    The CoAP context to register the event handler with.
 * @param hnd"
 *    The event handler to be registered. NULL if to be de-registered.
 */
void coap_register_event_handler(struct coap_context_t *context,
                            coap_event_handler_t hnd);

/**
 * @brief: Registers the function @p hnd as callback for events from the given
 *    CoAP context @p context. Any event handler that has previously been
 *    registered with @p context will be overwritten by this operation.
 *
 * @deprecated: Use coap_register_event_handler() instead.
 *
 * @param context:
 *    The CoAP context to register the event handler with.
 * @param hnd:
 *    The event handler to be registered.
 */
COAP_DEPRECATED
void coap_set_event_handler(struct coap_context_t *context,
                            coap_event_handler_t hnd);

/**
 * @brief: Clears the event handler registered with @p context.
 *
 * @deprecated: Use coap_register_event_handler() instead with NULL for hnd.
 *
 * @param context:
 *    The CoAP context whose event handler is to be removed.
 */
COAP_DEPRECATED
void coap_clear_event_handler(struct coap_context_t *context);

#endif /* COAP_EVENT_H */