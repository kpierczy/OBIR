/* ============================================================================================================
 *  File:
 *  Author: Olaf Bergmann
 *  Source: https://github.com/obgm/libcoap/tree/develop/include/coap2
 *  Modified by: Krzysztof Pierczyk
 *  Modified time: 2020-11-22 23:34:53
 *  Description:
 * 
 *      File contains API related to creation, analysis and manipulation CoAP PDUs (Protocol Data Units).
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
 * pdu.h -- CoAP message structure
 *
 * Copyright (C) 2010-2014 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see README for terms
 * of use.
 */

/**
 * @file pdu.h
 * @brief Pre-defined constants that reflect defaults for CoAP
 */

/* ------------------------------------------------------------------------------------------------------------ */


#ifndef COAP_PDU_H_
#define COAP_PDU_H_

#include <stdint.h>
#include "uri.h"

struct coap_session_t;


/* ------------------------------------------- [Macrodefinitions] --------------------------------------------- */

// CoAP default UDP port
#define COAP_DEFAULT_PORT  5683 

// Version of CoAP supported
#define COAP_DEFAULT_VERSION 1

// The default scheme for CoAP URIs
#define COAP_DEFAULT_SCHEME  "coap" 
// well-known resources URI
#define COAP_DEFAULT_URI_WELLKNOWN ".well-known/core"

// Default Max-Age (in seconds)
#define COAP_DEFAULT_MAX_AGE     60

// Default MTU (Maximum Transport Unit) (Excluding IP and UDP overhead)
#ifndef COAP_DEFAULT_MTU
#define COAP_DEFAULT_MTU 1152
#endif

// 8 MiB max-message-size plus some space for options
#ifndef COAP_DEFAULT_MAX_PDU_RX_SIZE
#define COAP_DEFAULT_MAX_PDU_RX_SIZE (8*1024*1024+256)
#endif

/* 1024 derived from RFC7252 4.6. Message Size max payload */
#ifndef COAP_DEBUG_BUF_SIZE
#define COAP_DEBUG_BUF_SIZE (8 + 1024 * 2)
#endif

// CoAP message types
#define COAP_MESSAGE_CON 0 /* confirmable message (requires ACK/RST) */
#define COAP_MESSAGE_NON 1 /* non-confirmable message (one-shot message) */
#define COAP_MESSAGE_ACK 2 /* used to acknowledge confirmable messages */
#define COAP_MESSAGE_RST 3 /* indicates error in received messages */

// CoAP request methods
#define COAP_REQUEST_GET    1
#define COAP_REQUEST_POST   2
#define COAP_REQUEST_PUT    3
#define COAP_REQUEST_DELETE 4
// (RFC 8132 :)
#define COAP_REQUEST_FETCH  5
#define COAP_REQUEST_PATCH  6
#define COAP_REQUEST_IPATCH 7

// The highest option number 
#define COAP_MAX_OPT 65535 

/*
 * @brief: CoAP option types 
 * 
 * @note: Be sure to update coap_option_check_critical() when adding options
 */
#define COAP_OPTION_IF_MATCH        1 /* C,     opaque,     0-8 B, (none)                         */
#define COAP_OPTION_URI_HOST        3 /* C,     String,   1-255 B,  destination address           */
#define COAP_OPTION_ETAG            4 /* E,     opaque,     1-8 B, (none)                         */
#define COAP_OPTION_IF_NONE_MATCH   5 /* -,      empty,       0 B, (none)                         */
#define COAP_OPTION_URI_PORT        7 /* C,       uint,     0-2 B,  destination port              */
#define COAP_OPTION_LOCATION_PATH   8 /* E,     String,   0-255 B,  -                             */
#define COAP_OPTION_URI_PATH       11 /* C,     String,   0-255 B, (none)                         */
#define COAP_OPTION_CONTENT_FORMAT 12 /* E,       uint,     0-2 B, (none)                         */
#define COAP_OPTION_MAXAGE         14 /* E,       uint,    0--4 B,  60 Seconds                    */
#define COAP_OPTION_URI_QUERY      15 /* C,     String,   1-255 B, (none)                         */
#define COAP_OPTION_ACCEPT         17 /* C,       uint,     0-2 B, (none)                         */
#define COAP_OPTION_LOCATION_QUERY 20 /* E,     String,   0-255 B, (none)                         */
#define COAP_OPTION_SIZE2          28 /* E,       uint,     0-4 B, (none)                         */
#define COAP_OPTION_PROXY_URI      35 /* C,     String,  1-1034 B, (none)                         */
#define COAP_OPTION_PROXY_SCHEME   39 /* C,     String,   1-255 B, (none)                         */
#define COAP_OPTION_SIZE1          60 /* E,       uint,     0-4 B, (none)                         */
#define COAP_OPTION_OBSERVE         6 /* E, empty/uint, 0 B/0-3 B, (none)              (RFC 7641) */
#define COAP_OPTION_BLOCK2         23 /* C,       uint,    0--3 B, (none)              (RFC 7959) */
#define COAP_OPTION_BLOCK1         27 /* C,       uint,    0--3 B, (none)              (RFC 7959) */
#define COAP_OPTION_NORESPONSE    258 /* N,       uint,    0--1 B,  0                  (RFC 7967) */
// Synonymous options
#define COAP_OPTION_CONTENT_TYPE \
    COAP_OPTION_CONTENT_FORMAT
#define COAP_OPTION_SUBSCRIPTION \
    COAP_OPTION_OBSERVE

/* @note: Response codes are encoded to base 32, i.e. the three upper bits determine 
 *    the response class while the remaining five fine-grained information specific 
 *    to that class.
 */

// Encodes integer response code value into the 3-5 bits format
#define COAP_RESPONSE_CODE(N)  (((N)/100 << 5) | (N)%100)
// Determines the class of response code C (i.e value of the upper three bits)
#define COAP_RESPONSE_CLASS(C) (((C) >> 5) & 0xFF)
// Encodes integer signaling code value into the 3-5 bits format
#define COAP_SIGNALING_CODE(N) (((N)/100 << 5) | (N)%100)

// Maximum length of error phrase
#define COAP_ERROR_PHRASE_LENGTH 32

// PDUs' codes
#define COAP_RESPONSE_200       COAP_RESPONSE_CODE(200)  /* 2.00 OK                       */
#define COAP_RESPONSE_201       COAP_RESPONSE_CODE(201)  /* 2.01 Created                  */
#define COAP_RESPONSE_304       COAP_RESPONSE_CODE(203)  /* 2.03 Valid                    */
#define COAP_RESPONSE_400       COAP_RESPONSE_CODE(400)  /* 4.00 Bad Request              */
#define COAP_RESPONSE_404       COAP_RESPONSE_CODE(404)  /* 4.04 Not Found                */
#define COAP_RESPONSE_405       COAP_RESPONSE_CODE(405)  /* 4.05 Method Not Allowed       */
#define COAP_RESPONSE_415       COAP_RESPONSE_CODE(415)  /* 4.15 Unsupported Media Type   */
#define COAP_RESPONSE_500       COAP_RESPONSE_CODE(500)  /* 5.00 Internal Server Error    */
#define COAP_RESPONSE_501       COAP_RESPONSE_CODE(501)  /* 5.01 Not Implemented          */
#define COAP_RESPONSE_503       COAP_RESPONSE_CODE(503)  /* 5.03 Service Unavailable      */
#define COAP_RESPONSE_504       COAP_RESPONSE_CODE(504)  /* 5.04 Gateway Timeout          */
#define COAP_RESPONSE_X_242     COAP_RESPONSE_CODE(402)  /* Critical Option not supported */

#define COAP_SIGNALING_CSM     COAP_SIGNALING_CODE(701)
#define COAP_SIGNALING_PING    COAP_SIGNALING_CODE(702)
#define COAP_SIGNALING_PONG    COAP_SIGNALING_CODE(703)
#define COAP_SIGNALING_RELEASE COAP_SIGNALING_CODE(704)
#define COAP_SIGNALING_ABORT   COAP_SIGNALING_CODE(705)

// Applies to COAP_SIGNALING_CSM
#define COAP_SIGNALING_OPTION_MAX_MESSAGE_SIZE 2
#define COAP_SIGNALING_OPTION_BLOCK_WISE_TRANSFER 4
// Applies to COAP_SIGNALING_PING / COAP_SIGNALING_PONG
#define COAP_SIGNALING_OPTION_CUSTODY 2
// Applies to COAP_SIGNALING_RELEASE
#define COAP_SIGNALING_OPTION_ALTERNATIVE_ADDRESS 2
#define COAP_SIGNALING_OPTION_HOLD_OFF 4
// Applies to COAP_SIGNALING_ABORT
#define COAP_SIGNALING_OPTION_BAD_CSM_OPTION 2

// CoAP media type encoding
#define COAP_MEDIATYPE_TEXT_PLAIN                 0 /* text/plain (UTF-8)       */
#define COAP_MEDIATYPE_APPLICATION_LINK_FORMAT   40 /* application/link-format  */
#define COAP_MEDIATYPE_APPLICATION_XML           41 /* application/xml          */
#define COAP_MEDIATYPE_APPLICATION_OCTET_STREAM  42 /* application/octet-stream */
#define COAP_MEDIATYPE_APPLICATION_RDF_XML       43 /* application/rdf+xml      */
#define COAP_MEDIATYPE_APPLICATION_EXI           47 /* application/exi          */
#define COAP_MEDIATYPE_APPLICATION_JSON          50 /* application/json         */
#define COAP_MEDIATYPE_APPLICATION_CBOR          60 /* application/cbor         */

// Content formats from RFC 8152
#define COAP_MEDIATYPE_APPLICATION_COSE_SIGN     98 /* application/cose; cose-type="cose-sign"     */
#define COAP_MEDIATYPE_APPLICATION_COSE_SIGN1    18 /* application/cose; cose-type="cose-sign1"    */
#define COAP_MEDIATYPE_APPLICATION_COSE_ENCRYPT  96 /* application/cose; cose-type="cose-encrypt"  */
#define COAP_MEDIATYPE_APPLICATION_COSE_ENCRYPT0 16 /* application/cose; cose-type="cose-encrypt0" */
#define COAP_MEDIATYPE_APPLICATION_COSE_MAC      97 /* application/cose; cose-type="cose-mac"      */
#define COAP_MEDIATYPE_APPLICATION_COSE_MAC0     17 /* application/cose; cose-type="cose-mac0"     */

#define COAP_MEDIATYPE_APPLICATION_COSE_KEY     101 /* application/cose-key     */
#define COAP_MEDIATYPE_APPLICATION_COSE_KEY_SET 102 /* application/cose-key-set */

// Content formats from RFC 8428
#define COAP_MEDIATYPE_APPLICATION_SENML_JSON   110 /* application/senml+json  */
#define COAP_MEDIATYPE_APPLICATION_SENSML_JSON  111 /* application/sensml+json */
#define COAP_MEDIATYPE_APPLICATION_SENML_CBOR   112 /* application/senml+cbor  */
#define COAP_MEDIATYPE_APPLICATION_SENSML_CBOR  113 /* application/sensml+cbor */
#define COAP_MEDIATYPE_APPLICATION_SENML_EXI    114 /* application/senml-exi   */
#define COAP_MEDIATYPE_APPLICATION_SENSML_EXI   115 /* application/sensml-exi  */
#define COAP_MEDIATYPE_APPLICATION_SENML_XML    310 /* application/senml+xml   */
#define COAP_MEDIATYPE_APPLICATION_SENSML_XML   311 /* application/sensml+xml  */

/**
 * @brief: Any media type.
 * 
 * @note: Identifiers for registered media types are in the range 0-65535. We
 *    use an unallocated type here and hope for the best. 
 */
#define COAP_MEDIATYPE_ANY 0xff

// Invalid transaction id
#define COAP_INVALID_TID -1

/**
 * @brief: Indicates that a response is suppressed. This will occur for error
 *    responses if the request was received via IP multicast.
 */
#define COAP_DROPPED_RESPONSE -2

/**
 * @brief: Indicates that a PDU has been delayed
 */
#define COAP_PDU_DELAYED -3

/**
 * @brief: Option code equal to 0b1111 indicates that the option list in a CoAP
 *   message is limited by 0b11110000 marker.
 * 
 */
#define COAP_OPT_LONG 0x0F

// End marker
#define COAP_OPT_END 0xF0

// Payload marker
#define COAP_PAYLOAD_START 0xFF 

// PDU recognition macros
#define COAP_PDU_IS_EMPTY(pdu)     ((pdu)->code == 0)
#define COAP_PDU_IS_REQUEST(pdu)   (!COAP_PDU_IS_EMPTY(pdu) && (pdu)->code < 32)
#define COAP_PDU_IS_RESPONSE(pdu)  ((pdu)->code >= 64 && (pdu)->code < 224)
#define COAP_PDU_IS_SIGNALING(pdu) ((pdu)->code >= 224)

// Header sizes for UDP protocols
#define COAP_PDU_MAX_UDP_HEADER_SIZE 4

// Protocols idntifiers for PDUs
#define COAP_PROTO_NONE 0
#define COAP_PROTO_UDP  1


/* -------------------------------------------- [Data structures] --------------------------------------------- */

/**
 * @brief: CoAP transaction id, i.e. a hash value built from the remote transport address
 *    and the message id of a CoAP PDU. Valid transaction ids are greater or equal zero.
 */
typedef int coap_tid_t;

/**
 * @brief: structure for CoAP PDUs
 *
 *    Memory layout is:
 *    <---header--->|<---token---><---options--->0xff<---payload--->
 * 
 * @note: header is addressed with a negative offset to token (using @attr token); its 
 *    maximum size is max_hdr_size.
 * @note: options starts at @attr token + @attr token_length
 * @note: payload starts at @attr data; its length is @attr used_size - (@attr data - @attr token)
 */
typedef struct coap_pdu_t {
  
    // Message type
    uint8_t type;
    // Request method (value 1--10) or response code (value 40-255)
    uint8_t code;

    // transaction id, if any (in regular host byte order)
    uint16_t tid;
    // highest option number
    uint16_t max_delta;

    // Space reserved for protocol-specific header
    uint8_t max_hdr_size;
    // Actaul size used for protocol-specific header
    uint8_t hdr_size;
    // Length of Token
    uint8_t token_length;
    
    // Storage allocated for token, options and payload
    size_t alloc_size;
    // Bytes used for storage for token, options and payload
    size_t used_size;
    // Maximum size for token, options and payload (zero for variable size pdu)
    size_t max_size;
    
    // First byte of token, if any, or options
    uint8_t *token;
    // First byte of payload, if any
    uint8_t *data;

} coap_pdu_t;

/**
 * @brief: Type representing transport layer protocl
 * 
 */
typedef uint8_t coap_proto_t;


/* ----------------------------------------------- [Functions] ------------------------------------------------ */

/**
 * @param code:
 *    the response code for which the literal phrase should be retrieved.
 * @return:
 *    a human-readable, '\0'-ended response phrase for the specified CoAP 
 *    response @p code on success
 *    NULL if reponse phrase has not been found
 */
const char *coap_response_phrase(unsigned char code);

/**
 * @brief: Creates a new CoAP PDU with at least enough storage space for the given @p size
 *    maximum message size. 
 *
 * @param type:
 *    the type of the PDU (one of: COAP_MESSAGE_CON, COAP_MESSAGE_NON, COAP_MESSAGE_ACK,
 *    COAP_MESSAGE_RST).
 * @param code:
 *    the message code
 * @param tid:
 *    the transcation id to set or 0 if unknown / not applicable
 * @param size:
 *    the maximum allowed number of byte for the message
 * @returns:
 *    a pointer to the new PDU object on success
 *    NULL on error
 * 
 * @note: The storage allocated for the result must be released with coap_delete_pdu() 
 *    if coap_send() is not called.
 */
coap_pdu_t *coap_pdu_init(
    uint8_t type,
    uint8_t code, 
    uint16_t tid, 
    size_t size
);

/**
 * @brief: Dynamically grows the size of @p pdu to @p new_size. The new size must not exceed
 *    the PDU's configure maximum size.
 *
 * @param pdu:
 *    the PDU to resize
 * @param new_size:
 *    the new size in bytes
 * @returns:
 *    1 if the operation succeeded
 *    0 on failure.
 */
int coap_pdu_resize(coap_pdu_t *pdu, size_t new_size);

/**
 * @brief: Clears any contents from @p pdu and resets @p pdu->used_size and @p pdu->data pointers.
 *    @p pdu->max_size is set to @p size, any other field is set to @c 0.
 * 
 * @param pdu:
 *    PDU to clear 
 * @param size:
 *     @p pdu's desired max_size
 * 
 * @note: @p pdu must be a valid pointer to a coap_pdu_t object created e.g. by coap_pdu_init().
 */
void coap_pdu_clear(coap_pdu_t *pdu, size_t size);

/**
 * @brief: Creates a new CoAP PDU for the @p session.
 * 
 * @param session:
 *    session associated with created PDU
 * @returns:
 *    new PDU on success
 *    NULL on failure
 */
coap_pdu_t *coap_new_pdu(const struct coap_session_t *session);

/**
 * @brief: Dispose of an CoAP PDU and frees associated storage. In general you should not call
 *    this function directly. When a PDU is sent with coap_send(), coap_delete_pdu() will be
 *    called automatically for you.
 * 
 * @param pdu:
 *    PDU to free
 */
void coap_delete_pdu(coap_pdu_t *pdu);

/**
 * @brief: Decode the protocol specific header for the specified PDU.
 * 
 * @param pdu:
 *    a newly received PDU
 * @param proto:
 *    the target wire protocol
 * @returns:
 *    1 for success 
 *    0 on error
 */
int coap_pdu_parse_header(
    coap_pdu_t *pdu,
    coap_proto_t proto
);

/**
 * @brief: Verify consistency in the given CoAP PDU structure and locate the data.
 *   This function only parses the token and options, up to the payload start marker.
 *
 * @param pdu:
 *    the PDU structure to
 * @returns:
 *    1 on success 
 *    0 on error
 */
int coap_pdu_parse_opt(coap_pdu_t *pdu);

/**
 * @brief: Parses @p data into the CoAP PDU structure given in @p result. The target pdu
 *    must be large enough to hold parsed data.
 *
 * @param proto:
 *    session's protocol
 * @param data:
 *    the raw data to parse as CoAP PDU
 * @param length:
 *    the actual size of @p data.
 * @param pdu:
 *    the PDU structure to fill
 * @returns:
 *    1 on success 
 *    0 on error.
 * 
 * @note: The structure must provide space to hold at least the token and options part of
 *    the message.
 */
int coap_pdu_parse(
    coap_proto_t proto,
    const uint8_t *data,
    size_t length,
    coap_pdu_t *pdu
);

/**
 * @brief: Adds token of length @p len to @p pdu. Adding the token destroys any following contents
 *    of the pdu. Hence options and data must be added after coap_add_token() has been called. 
 *    In @p pdu length is set to @p len + @c 4, and max_delta is set to @c 0. This function returns
 *    0 on error or a value greater than zero on success.
 *
 * @param pdu:
 *    the PDU where the token is to be added
 * @param len:
 *    the length of the new token
 * @param data:
 *    the token to add
 * @returns:
 *    a value greater than zero on success
 *    0 on error
 */
int coap_add_token(
    coap_pdu_t *pdu,
    size_t len,
    const uint8_t *data
);

/**
 * @brief: Adds option of given type to pdu that is passed as first parameter. It destroys the
 *    PDU's data, so coap_add_data() must be called after all options have been added.
 *    As coap_add_token() destroys the options following the token, the token must be added
 *    before coap_add_option() is called.
 * 
 * @param pdu:
 *    PDU to add options to 
 * @param type:
 *     option's type
 * @param len:
 *     length of the
 * @param data:
 *     option's value data buffer
 * @returns:
 *     the number of bytes written on success
 *     0 on error
 */
size_t coap_add_option(
    coap_pdu_t *pdu,
    uint16_t type,
    size_t len,
    const uint8_t *data
);

/**
 * @brief: Adds option of given type to @p pdu that is passed as first parameter, but does
 *    not write a value. It works like @f coap_add_option with respect to calling sequence
 *    (i.e. after token and before data).
 * 
 * @param pdu:
 *    PDU to write option to
 * @param type:
 *    option's type
 * @param len:
 *    length of the option's value
 * @returns:
 *     a memory address to which the option data has to be written before
 *     the PDU can be sent on success
 *     NULL on error.
 */
uint8_t *coap_add_option_later(
    coap_pdu_t *pdu,
    uint16_t type,
    size_t len
);

/**
 * @brief: Adds given @p data to the @p pdu.
 * 
 * @param pdu:
 *    PDU to add data to 
 * @param len:
 *     data's length
 * @param data:
 *     data buffer
 * @returns:
 *    value unequal to NULL on success
 *    0 on error
 * 
 * @note: The PDU's data is destroyed by @f coap_add_option(). @f coap_add_data() must be called
 *    only once per PDU, otherwise the result is undefined.
 */
int coap_add_data(
    coap_pdu_t *pdu,
    size_t len,
    const uint8_t *data
);

/**
 * @brief: Adds given @p data to the @p pdu but does not copyt it. The actual data must be copied at
 *    the returned location.
 *
 * @param pdu:
 *    PDU to add data to 
 * @param len:
 *     data's length
 * @returns:
 *    start address of the @p pdu's data buffer
 *    NULL on error
 * 
 * @note: The PDU's data is destroyed by @f coap_add_option(). @f coap_add_data() must be called
 *    only once per PDU, otherwise the result is undefined.
 */
uint8_t *coap_add_data_after(
    coap_pdu_t *pdu, 
    size_t len
);

/**
 * @brief: Retrieves the length and data pointer of specified PDU. 
 * 
 * @returns:
 *    1 if @p *len and @p *data have correct values.
 *    0 on error
 * 
 * @note: data buffer in the @p pdu is destroyed with the pdu.
 */
int coap_get_data(
    const coap_pdu_t *pdu,
    size_t *len,
    uint8_t **data
);

/**
 * @brief: Compose the protocol specific header for the specified PDU.
 * 
 * @param pdu:
 *    a newly composed PDU
 * @param proto:
 *    the target wire protocol
 * 
 * @returns:
 *    number of header bytes prepended before pdu->token on success
 *    0 on error
 */
size_t coap_pdu_encode_header(
    coap_pdu_t *pdu,
    coap_proto_t proto
);

#endif /* COAP_PDU_H_ */
