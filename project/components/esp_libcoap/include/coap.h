/* ============================================================================================================
 *  File: coap.h
 *  Author: Olaf Bergmann
 *  License: BSD
 *  Source: https://github.com/obgm/libcoap/tree/develop
 *  Modified by: Krzysztof Pierczyk
 *  Modified time: 2020-11-20 15:23:10
 *  Description:
 * 
 *      Consolidation of the library's header files
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

/* Modify head file implementation for ESP32 platform.
 *
 * Uses libcoap software implementation for failover when concurrent
 * define operations are in use.
 *
 * coap.h -- main header file for CoAP stack of libcoap
 *
 * Copyright (C) 2010-2012,2015-2016 Olaf Bergmann <bergmann@tzi.org>
 *               2015 Carsten Schoenert <c.schoenert@t-online.de>
 *
 * Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
 *
 * This file is part of the CoAP library libcoap. Please see README for terms
 * of use.
 */

/* ------------------------------------------------------------------------------------------------------------ */

#ifndef _COAP_H_
#define _COAP_H_

#include "libcoap.h"

#include "address.h"
#include "bits.h"
#include "block.h"
#include "coap_io.h"
#include "coap_time.h"
#include "coap_debug.h"
#include "encode.h"
#include "mem.h"
#include "net.h"
#include "option.h"
#include "pdu.h"
#include "prng.h"
#include "resource.h"
#include "str.h"
#include "subscribe.h"
#include "uri.h"

#endif /* _COAP_H_ */
