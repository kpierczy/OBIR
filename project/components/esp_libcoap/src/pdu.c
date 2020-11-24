/* pdu.c -- CoAP message structure
 *
 * Copyright (C) 2010--2016 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see
 * README for terms of use.
 */

#include "coap_config.h"

# include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "libcoap.h"
#include "coap_debug.h"
#include "pdu.h"
#include "option.h"
#include "encode.h"
#include "mem.h"
#include "coap_session.h"

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

void
coap_pdu_clear(coap_pdu_t *pdu, size_t size) {
  assert(pdu);
  assert(pdu->token);
  
  if (pdu->alloc_size > size)
    pdu->alloc_size = size;
  pdu->type = 0;
  pdu->code = 0;
  
  pdu->token_length = 0;
  pdu->tid = 0;
  pdu->max_delta = 0;
  pdu->max_size = size;
  pdu->used_size = 0;
  pdu->data = NULL;
}

coap_pdu_t *
coap_pdu_init(uint8_t type, uint8_t code, uint16_t tid, size_t size) {
  coap_pdu_t *pdu;

  pdu = (coap_pdu_t *) coap_malloc(sizeof(coap_pdu_t));
  if (!pdu) return NULL;

  uint8_t *buf;
  pdu->alloc_size = min(size, 256);
  buf = (uint8_t*) coap_malloc(pdu->alloc_size + COAP_HEADER_SIZE);
  if (buf == NULL) {
    coap_free(pdu);
    return NULL;
  }
  pdu->token = buf + COAP_HEADER_SIZE;

  coap_pdu_clear(pdu, size);
  pdu->tid = tid;
  pdu->type = type;
  pdu->code = code;
  return pdu;
}

coap_pdu_t *
coap_new_pdu(const struct coap_session_t *session) {
  coap_pdu_t *pdu = coap_pdu_init(0, 0, 0, coap_session_max_pdu_size(session));
#ifndef NDEBUG
  if (!pdu)
    coap_log(LOG_CRIT, "coap_new_pdu: cannot allocate memory for new PDU\n");
#endif
  return pdu;
}

void
coap_delete_pdu(coap_pdu_t *pdu) {
  if (pdu != NULL) {

    if (pdu->token != NULL)
      coap_free(pdu->token - COAP_HEADER_SIZE);
    coap_free(pdu);
  }
}

int
coap_pdu_resize(coap_pdu_t *pdu, size_t new_size) {
  if (new_size > pdu->alloc_size) {

    uint8_t *new_hdr;
    size_t offset;

    if (pdu->max_size && new_size > pdu->max_size) {
      coap_log(LOG_WARNING, "coap_pdu_resize: pdu too big\n");
      return 0;
    }

    if (pdu->data != NULL) {
      assert(pdu->data > pdu->token);
      offset = pdu->data - pdu->token;
    } else {
      offset = 0;
    }
    new_hdr = (uint8_t*)realloc(pdu->token - COAP_HEADER_SIZE, new_size + COAP_HEADER_SIZE);
    if (new_hdr == NULL) {
      coap_log(LOG_WARNING, "coap_pdu_resize: realloc failed\n");
      return 0;
    }
    pdu->token = new_hdr + COAP_HEADER_SIZE;
    if (offset > 0)
      pdu->data = pdu->token + offset;
    else
      pdu->data = NULL;

  }
  pdu->alloc_size = new_size;
  return 1;
}

static int
coap_pdu_check_resize(coap_pdu_t *pdu, size_t size) {
  if (size > pdu->alloc_size) {
    size_t new_size = max(256, pdu->alloc_size * 2);
    while (size > new_size)
      new_size *= 2;
    if (pdu->max_size && new_size > pdu->max_size) {
      new_size = pdu->max_size;
      if (new_size < size)
        return 0;
    }
    if (!coap_pdu_resize(pdu, new_size))
      return 0;
  }
  return 1;
}

int
coap_add_token(coap_pdu_t *pdu, size_t len, const uint8_t *data) {
  /* must allow for pdu == NULL as callers may rely on this */
  if (!pdu || len > 8)
    return 0;

  if (pdu->used_size) {
    coap_log(LOG_WARNING,
             "coap_add_token: The token must defined first. Token ignored\n");
    return 0;
  }
  if (!coap_pdu_check_resize(pdu, len))
    return 0;
  pdu->token_length = (uint8_t)len;
  if (len)
    memcpy(pdu->token, data, len);
  pdu->max_delta = 0;
  pdu->used_size = len;
  pdu->data = NULL;

  return 1;
}

/* FIXME: de-duplicate code with coap_add_option_later */
size_t
coap_add_option(coap_pdu_t *pdu, uint16_t type, size_t len, const uint8_t *data) {
  size_t optsize;
  coap_opt_t *opt;

  assert(pdu);
  pdu->data = NULL;

  if (type < pdu->max_delta) {
    coap_log(LOG_WARNING,
             "coap_add_option: options are not in correct order\n");
    return 0;
  }

  if (!coap_pdu_check_resize(pdu,
      pdu->used_size + coap_opt_encode_size(type - pdu->max_delta, len)))
    return 0;

  opt = pdu->token + pdu->used_size;

  /* encode option and check length */
  optsize = coap_opt_encode(opt, pdu->alloc_size - pdu->used_size,
                            type - pdu->max_delta, data, len);

  if (!optsize) {
    coap_log(LOG_WARNING, "coap_add_option: cannot add option\n");
    /* error */
    return 0;
  } else {
    pdu->max_delta = type;
    pdu->used_size += optsize;
  }

  return optsize;
}

/* FIXME: de-duplicate code with coap_add_option */
uint8_t*
coap_add_option_later(coap_pdu_t *pdu, uint16_t type, size_t len) {
  size_t optsize;
  coap_opt_t *opt;

  assert(pdu);
  pdu->data = NULL;

  if (type < pdu->max_delta) {
    coap_log(LOG_WARNING,
             "coap_add_option: options are not in correct order\n");
    return NULL;
  }

  if (!coap_pdu_check_resize(pdu,
      pdu->used_size + coap_opt_encode_size(type - pdu->max_delta, len)))
    return 0;

  opt = pdu->token + pdu->used_size;

  /* encode option and check length */
  optsize = coap_opt_encode(opt, pdu->alloc_size - pdu->used_size,
                            type - pdu->max_delta, NULL, len);

  if (!optsize) {
    coap_log(LOG_WARNING, "coap_add_option: cannot add option\n");
    /* error */
    return NULL;
  } else {
    pdu->max_delta = type;
    pdu->used_size += (uint16_t)optsize;
  }

  return opt + optsize - len;
}

int
coap_add_data(coap_pdu_t *pdu, size_t len, const uint8_t *data) {
  if (len == 0) {
    return 1;
  } else {
    uint8_t *payload = coap_add_data_after(pdu, len);
    if (payload != NULL)
      memcpy(payload, data, len);
    return payload != NULL;
  }
}

uint8_t *
coap_add_data_after(coap_pdu_t *pdu, size_t len) {
  assert(pdu);
  assert(pdu->data == NULL);

  pdu->data = NULL;

  if (len == 0)
    return NULL;

  if (!coap_pdu_resize(pdu, pdu->used_size + len + 1))
    return 0;
  pdu->token[pdu->used_size++] = COAP_PAYLOAD_START;
  pdu->data = pdu->token + pdu->used_size;
  pdu->used_size += len;
  return pdu->data;
}

int
coap_get_data(const coap_pdu_t *pdu, size_t *len, uint8_t **data) {
  assert(pdu);
  assert(len);
  assert(data);

  *data = pdu->data;
  if(pdu->data == NULL) {
     *len = 0;
     return 0;
  }

  *len = pdu->used_size - (pdu->data - pdu->token);

  return 1;
}

#ifndef SHORT_ERROR_RESPONSE
typedef struct {
  unsigned char code;
  const char *phrase;
} error_desc_t;

/* if you change anything here, make sure, that the longest string does not
 * exceed COAP_ERROR_PHRASE_LENGTH. */
error_desc_t coap_error[] = {
  { COAP_RESPONSE_CODE(201), "Created" },
  { COAP_RESPONSE_CODE(202), "Deleted" },
  { COAP_RESPONSE_CODE(203), "Valid" },
  { COAP_RESPONSE_CODE(204), "Changed" },
  { COAP_RESPONSE_CODE(205), "Content" },
  { COAP_RESPONSE_CODE(231), "Continue" },
  { COAP_RESPONSE_CODE(400), "Bad Request" },
  { COAP_RESPONSE_CODE(401), "Unauthorized" },
  { COAP_RESPONSE_CODE(402), "Bad Option" },
  { COAP_RESPONSE_CODE(403), "Forbidden" },
  { COAP_RESPONSE_CODE(404), "Not Found" },
  { COAP_RESPONSE_CODE(405), "Method Not Allowed" },
  { COAP_RESPONSE_CODE(406), "Not Acceptable" },
  { COAP_RESPONSE_CODE(408), "Request Entity Incomplete" },
  { COAP_RESPONSE_CODE(412), "Precondition Failed" },
  { COAP_RESPONSE_CODE(413), "Request Entity Too Large" },
  { COAP_RESPONSE_CODE(415), "Unsupported Content-Format" },
  { COAP_RESPONSE_CODE(500), "Internal Server Error" },
  { COAP_RESPONSE_CODE(501), "Not Implemented" },
  { COAP_RESPONSE_CODE(502), "Bad Gateway" },
  { COAP_RESPONSE_CODE(503), "Service Unavailable" },
  { COAP_RESPONSE_CODE(504), "Gateway Timeout" },
  { COAP_RESPONSE_CODE(505), "Proxying Not Supported" },
  { 0, NULL }                        /* end marker */
};

const char *
coap_response_phrase(unsigned char code) {
  int i;
  for (i = 0; coap_error[i].code; ++i) {
    if (coap_error[i].code == code)
      return coap_error[i].phrase;
  }
  return NULL;
}
#endif

/**
 * Advances *optp to next option if still in PDU. This function
 * returns the number of bytes opt has been advanced or @c 0
 * on error.
 */
static size_t
next_option_safe(coap_opt_t **optp, size_t *length) {
  coap_option_t option;
  size_t optsize;

  assert(optp); assert(*optp);
  assert(length);

  optsize = coap_opt_parse(*optp, *length, &option);
  if (optsize) {
    assert(optsize <= *length);

    *optp += optsize;
    *length -= optsize;
  }

  return optsize;
}

int
coap_pdu_parse_header(coap_pdu_t *pdu) {
  uint8_t *hdr = pdu->token - COAP_HEADER_SIZE;
  
  if ((hdr[0] >> 6) != COAP_DEFAULT_VERSION) {
    coap_log(LOG_DEBUG, "coap_pdu_parse: UDP version not supported\n");
    return 0;
  }
  pdu->type = (hdr[0] >> 4) & 0x03;
  pdu->token_length = hdr[0] & 0x0f;
  pdu->code = hdr[1];
  pdu->tid = (uint16_t)hdr[2] << 8 | hdr[3];
  if (pdu->token_length > pdu->alloc_size) {
    /* Invalid PDU provided - not wise to assert here though */
    coap_log(LOG_DEBUG, "coap_pdu_parse: PDU header token size broken\n");
    pdu->token_length = (uint8_t)pdu->alloc_size;
    return 0;
  }
  return 1;
}

int
coap_pdu_parse_opt(coap_pdu_t *pdu) {

  /* sanity checks */
  if (pdu->code == 0) {
    if (pdu->used_size != 0 || pdu->token_length) {
      coap_log(LOG_DEBUG, "coap_pdu_parse: empty message is not empty\n");
      return 0;
    }
  }

  if (pdu->token_length > pdu->used_size || pdu->token_length > 8) {
    coap_log(LOG_DEBUG, "coap_pdu_parse: invalid Token\n");
    return 0;
  }

  if (pdu->code == 0) {
    /* empty packet */
    pdu->used_size = 0;
    pdu->data = NULL;
  } else {
    /* skip header + token */
    coap_opt_t *opt = pdu->token + pdu->token_length;
    size_t length = pdu->used_size - pdu->token_length;

    while (length > 0 && *opt != COAP_PAYLOAD_START) {
      if ( !next_option_safe( &opt, (size_t *)&length ) ) {
        coap_log(LOG_DEBUG, "coap_pdu_parse: missing payload start code\n");
        return 0;
      }
    }

    if (length > 0) {
      assert(*opt == COAP_PAYLOAD_START);
      opt++; length--;

      if (length == 0) {
        coap_log(LOG_DEBUG,
                 "coap_pdu_parse: message ending in payload start marker\n");
        return 0;
      }
    }
    if (length > 0)
                pdu->data = (uint8_t*)opt;
    else
      pdu->data = NULL;
  }

  return 1;
}

int
coap_pdu_parse(

               const uint8_t *data,
               size_t length,
               coap_pdu_t *pdu)
{

  if (length == 0)
    return 0;
  if (COAP_HEADER_SIZE > length)
    return 0;
  if (!coap_pdu_resize(pdu, length - COAP_HEADER_SIZE))
    return 0;
  memcpy(pdu->token - COAP_HEADER_SIZE, data, length);
  pdu->used_size = length - COAP_HEADER_SIZE;
  return coap_pdu_parse_header(pdu) && coap_pdu_parse_opt(pdu);
}

void
coap_pdu_encode_header(coap_pdu_t *pdu) {
    
    uint8_t* header = pdu->token - COAP_HEADER_SIZE;

    header[0] = COAP_DEFAULT_VERSION << 6
                   | pdu->type << 4
                   | pdu->token_length;
    header[1] = pdu->code;
    header[2] = (uint8_t)(pdu->tid >> 8);
    header[3] = (uint8_t)(pdu->tid);
}
