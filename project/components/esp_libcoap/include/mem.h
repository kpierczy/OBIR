/* ============================================================================================================
 *  File:
 *  Author: Olaf Bergmann
 *  Source: https://github.com/obgm/libcoap/tree/develop/include/coap2
 *  Modified by: Krzysztof Pierczyk
 *  Modified time: 2020-11-20 16:46:38
 *  Description:
 * 
 *      Constarined-devices, libcoap-specific memory allocation API.
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
 * mem.h -- CoAP memory handling
 *
 * Copyright (C) 2010-2011,2014-2015 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see README for terms
 * of use.
 */

/* ------------------------------------------------------------------------------------------------------------ */


#ifndef COAP_MEM_H_
#define COAP_MEM_H_

#include <stdlib.h>
#include <libcoap.h>


/* -------------------------------------------- [Data structures] --------------------------------------------- */

/**
 * @brief: Type specifiers for coap_malloc_type() calls. Memory objects can be typed to
 *    facilitate arrays of type objects to be used instead of dynamic memory management
 *    on constrained devices.
 */
typedef enum {
    COAP_STRING,
    COAP_ATTRIBUTE_NAME,
    COAP_ATTRIBUTE_VALUE,
    COAP_PACKET,
    COAP_NODE,
    COAP_CONTEXT,
    COAP_ENDPOINT,
    COAP_PDU,
    COAP_PDU_BUF,
    COAP_RESOURCE,
    COAP_RESOURCEATTR,
    COAP_SESSION,
    COAP_OPTLIST,
} coap_memory_tag_t;


/* ----------------------------------------------- [Functions] ------------------------------------------------ */

/**
 * @brief: Initializes libcoap's memory management. This function must be called once
 *    before coap_malloc() can be used on constrained devices.
 */
void coap_memory_init(void);

/**
 * @brief: Allocates a chunk of @p size bytes and returns a pointer to the newly allocated
 *    memory. The @p type is used to select the appropriate storage container on constrained
 *    devices. The storage allocated by coap_malloc_type() must be released with coap_free_type().
 *
 * @param type:
 *    the type of object to be stored
 * @param size:
 *    the number of bytes requested
 * @return:
 *    a pointer to the allocated storage or @c NULL on error
 */
void *coap_malloc_type(coap_memory_tag_t type, size_t size);

/**
 * @brief: Releases the memory that was allocated by coap_malloc_type(). The type tag @p
 *    type must be the same that was used for allocating the object pointed to by @p p.
 *
 * @param type:
 *    the type of the object to release
 * @param p:
 *    a pointer to memory that was allocated by coap_malloc_type()
 */
void coap_free_type(coap_memory_tag_t type, void *p);


/* ---------------------------------------- [Static-inline functions] ----------------------------------------- */

/**
 * @brief: Wrapper function to coap_malloc_type() for backwards compatibility.
 */
COAP_STATIC_INLINE void*
coap_malloc(size_t size) {
  return coap_malloc_type(COAP_STRING, size);
}

/**
 * @brief: Wrapper function to coap_free_type() for backwards compatibility.
 */
COAP_STATIC_INLINE void 
coap_free(void *object) {
  coap_free_type(COAP_STRING, object);
}

#endif /* COAP_MEM_H_ */
