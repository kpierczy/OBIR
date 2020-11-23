/* ============================================================================================================
 *  File:
 *  Author: Olaf Bergmann
 *  Source: https://github.com/obgm/libcoap/tree/develop/include/coap2
 *  Modified by: Krzysztof Pierczyk
 *  Modified time: 2020-11-23 02:50:10
 *  Description:
 *  Credits: 
 *
 *      This file is a modification of the original libcoap source file. Aim of the modification was to 
 *      provide cleaner, richer documented and ESP8266-optimised version of the library. Core API of the 
 *      project was not changed or expanded, although some elemenets (e.g. DTLS support) have been removed 
 *      due to lack of needings from the modifications' authors. 
 * 
 * ============================================================================================================ */


/* -------------------------------------------- [Original header] --------------------------------------------- */

/* mem.c -- CoAP memory handling
 *
 * Copyright (C) 2014--2015 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see
 * README for terms of use.
 */

/* ------------------------------------------------------------------------------------------------------------ */


#include "coap_config.h"

#include <assert.h>
#include <stdlib.h>
#include "libcoap.h"
#include "mem.h"
#include "coap_debug.h"


void coap_memory_init(void) {}


void *coap_malloc_type(coap_memory_tag_t type, size_t size){
  (void) type;
  return malloc(size);
}

void
coap_free_type(coap_memory_tag_t type, void *p) {
  (void)type;
  free(p);
}
