/* ============================================================================================================
 *  File:
 *  Author: Olaf Bergmann
 *  Source: https://github.com/obgm/libcoap
 *  Modified by: Krzysztof Pierczyk
 *  Modified time: 2020-11-26 18:35:47
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

/*
 * option.c -- helpers for handling options in CoAP PDUs
 *
 * Copyright (C) 2010-2013 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see
 * README for terms of use.
 */

/* ------------------------------------------------------------------------------------------------------------ */


# include <assert.h>
#include <stdio.h>
#include <string.h>

#include "libcoap.h"
#include "option.h"
#include "encode.h"
#include "coap_config.h"
#include "coap_debug.h"
#include "mem.h"
#include "utlist.h"


/* -------------------------------------------- [Macrofeinitions] --------------------------------------------- */

/**
 * @brief: Helper macro used to safely move @p option pointer forward 
 *    and update value of the remaining option buffer's length
 * 
 * @param option:
 *    option pointer to move forward
 * @param length:
 *    length of the option buffer that @p option points to 
 * @param step:
 *    number of steps by which @p option should be moved
 */
#define ADVANCE_OPT(option,length,step)                        \
    if ((length) < (step)) {                                   \
        coap_log(LOG_DEBUG, "cannot advance opt past end\n");  \
        return 0;                                              \
    } else {                                                   \
        (length) -= (step);                                    \
        (option) += (step);                                    \
    }

/**
 * @brief: Helper macro used to sequentially forward the option's pointer and return
 *    from the function using the macro when @p option pointer was moved beyond the last
 *    element of the buffer.
 */
#define ADVANCE_OPT_CHECK(option,length,step) \
    do {                                      \
        ADVANCE_OPT(option,length,step);      \
        if ((length) < 1)                     \
            return 0;                         \
    } while (0)


/**
 * @brief: Masks used to lift out an option's type from the option filter
 *    @t opt_filter 
 */
#define LONG_MASK ((1 << COAP_OPT_FILTER_LONG) - 1)
#define SHORT_MASK \
  (~LONG_MASK & ((1 << (COAP_OPT_FILTER_LONG + COAP_OPT_FILTER_SHORT)) - 1))


/* ---------------------------------------- [Global and static data] ------------------------------------------ */

/**
 * @brief: Local structure used for options filtering
 */
typedef struct {

    // Bit-wise mask used for options filtering
    uint16_t mask;

    // Filtered options
    uint16_t long_opts[COAP_OPT_FILTER_LONG];
    uint8_t short_opts[COAP_OPT_FILTER_SHORT];
    
} opt_filter;

/**
 * @brief: Operation specifiers for @fcoap_filter_op()
 */
enum filter_op_t { FILTER_SET, FILTER_CLEAR, FILTER_GET };


/* ----------------------------------------------- [Functions] ------------------------------------------------ */

size_t
coap_opt_parse(const coap_opt_t *opt, size_t length, coap_option_t *result) {

  const coap_opt_t *opt_start = opt; /* store where parsing starts  */

  assert(opt); assert(result);

  if (length < 1){
    return 0;
  }

  result->delta = (*opt & 0xf0) >> 4;
  result->length = *opt & 0x0f;

  switch (result->delta) {
      case 15: 
          coap_log(LOG_WARNING, "coap_opt_delta: illegal option delta\n");
          return 0;
      // If delta = 14, it's notation will be extended by 2 bytes
      case 14:
          result->delta = (opt[0] << 8) + opt[1] + 269;
          if (result->delta < 269) {
              coap_log(LOG_DEBUG, "coap_opt_delta: delta too large\n");
              return 0;
          }
          break;
      // If delta = 13, it's notation will be extended by 1 bytes
      case 13:
          result->delta += opt[0];
          break;
  }

  switch(result->delta) {
  case 15:
    if (*opt != COAP_PAYLOAD_START) {
      coap_log(LOG_DEBUG, "ignored reserved option delta 15\n");
    }
    return 0;
  case 14:
    /* Handle two-byte value: First, the MSB + 269 is stored as delta value.
     * After that, the option pointer is advanced to the LSB which is handled
     * just like case delta == 13. */
    ADVANCE_OPT_CHECK(opt,length,1);
    result->delta = ((*opt & 0xff) << 8) + 269;
    if (result->delta < 269) {
      coap_log(LOG_DEBUG, "delta too large\n");
      return 0;
    }
    /* fall through */
  case 13:
    ADVANCE_OPT_CHECK(opt,length,1);
    result->delta += *opt & 0xff;
    break;

  default:
    ;
  }

  switch(result->length) {
  case 15:
    coap_log(LOG_DEBUG, "found reserved option length 15\n");
    return 0;
  case 14:
    /* Handle two-byte value: First, the MSB + 269 is stored as delta value.
     * After that, the option pointer is advanced to the LSB which is handled
     * just like case delta == 13. */
    ADVANCE_OPT_CHECK(opt,length,1);
    result->length = ((*opt & 0xff) << 8) + 269;
    /* fall through */
  case 13:
    ADVANCE_OPT_CHECK(opt,length,1);
    result->length += *opt & 0xff;
    break;

  default:
    ;
  }

  result->delta = coap_opt_delta(opt_start);
  result->length = coap_opt_length(opt_start);

  /* ADVANCE_OPT() is correct here */
  ADVANCE_OPT(opt,length,1);
  /* opt now points to value, if present */

  result->value = opt;
  if (length < result->length) {
    coap_log(LOG_DEBUG, "invalid option length\n");
    return 0;
  }

#undef ADVANCE_OPT
#undef ADVANCE_OPT_CHECK

  return (opt + result->length) - opt_start;
}


coap_opt_iterator_t *coap_option_iterator_init(
    const coap_pdu_t *pdu, 
    coap_opt_iterator_t *opt_iter,
    const coap_opt_filter_t filter
) {
    assert(pdu);
    assert(pdu->token);
    assert(opt_iter);

    // Clear the iterator
    memset(opt_iter, 0, sizeof(coap_opt_iterator_t));

    //Initialize iterator to the first option in the PDU (if present)
    opt_iter->next_option = pdu->token + pdu->token_length;
    // Check whether options might be allocated to the PDU
    if (pdu->token + pdu->used_size <= opt_iter->next_option) {
        opt_iter->bad = 1;
        return NULL;
    }

    // Set maximum length of the option's are (possibly this region holds also payload)
    opt_iter->length = pdu->used_size - pdu->token_length;

    // Apply a filter to the iterator (if present)
    if (filter) {
        memcpy(opt_iter->filter, filter, sizeof(coap_opt_filter_t));
        opt_iter->filtered = 1;
    }

    return opt_iter;
}


COAP_STATIC_INLINE int opt_finished(coap_opt_iterator_t *opt_iter) {

    assert(opt_iter);

    if (opt_iter->length == 0 || opt_iter->next_option == NULL || *(opt_iter->next_option) == COAP_PAYLOAD_START)
        opt_iter->bad = 1;

    return opt_iter->bad;
}


coap_opt_t *coap_option_next(coap_opt_iterator_t *opt_iter) {

    assert(opt_iter);
    if (opt_finished(opt_iter))
        return NULL;

    // Next option (the one to be returned)
    coap_opt_t *current_opt = NULL;

    // Iterate over subsequent options and break when the first non-filtered-out one appears
    while (true) {

        /** 
         * @note: @p opt_iter->option always points to the next option to be delivered;
         *    when opt_finished() filters out any bad state of the iterator, we can assume
         *    that @p opt_iter->option is valid. 
         */

        // Get the next option
        current_opt = opt_iter->next_option;

        // Parse an option from byte-vector to the dedicated structure
        coap_option_t option;
        size_t optsize = coap_opt_parse(opt_iter->next_option, opt_iter->length, &option);

        // Advance internal pointer to next option, if parsing succeeded
        if (optsize) {
            opt_iter->next_option += optsize;
            opt_iter->length -= optsize;
            opt_iter->type += option.delta;
        } else {
            opt_iter->bad = 1;
            return NULL;
        }

        /**
         * @brief: Exit the while loop when:
         *   - no filtering is done at all
         *   - the filter matches for the current option
         *   - the filter is too small for the current option number
         */

        int filter_found = 0;

        // Filterring off
        if (!opt_iter->filtered)
            break;
        // Filter matched
        else if((filter_found = coap_option_filter_get(opt_iter->filter, opt_iter->type) > 0))
            break;
        // Filtering error
        else if (filter_found < 0) {
            opt_iter->bad = 1;
            return NULL;
        }
    }

    return current_opt;
}


coap_opt_t *coap_check_option(
    coap_pdu_t *pdu, 
    uint16_t type,
    coap_opt_iterator_t *opt_iter
) {
    coap_opt_filter_t opt_filter;

    // Reset the filter to the given @p type
    coap_option_filter_clear(opt_filter);
    coap_option_filter_set(opt_filter, type);

    // Bind the iterator to the @p pdu's options
    coap_option_iterator_init(pdu, opt_iter, opt_filter);

    // Try to parse the first option in the @p pdu
    return coap_option_next(opt_iter);
}


uint16_t coap_opt_delta(const coap_opt_t *opt) {
  
    // Shortes options have their delta encoded onto higher 4 bits of the 1st byte
    uint16_t delta = (*opt++ & 0xf0) >> 4;

    switch (delta) {
        case 15: 
            coap_log(LOG_WARNING, "coap_opt_delta: illegal option delta\n");
            return 0;
        // If delta = 14, it's notation will be extended by 2 bytes
        case 14:
            delta = (opt[0] << 8) + 269;
            ++opt;
            if (delta < 269) {
                coap_log(LOG_DEBUG, "coap_opt_delta: delta too large\n");
                return 0;
            }
        // If delta = 13, it's notation will be extended by 1 bytes
        case 13:
            delta += opt[0];
            break;
    }

    return delta;
}


uint16_t coap_opt_length(const coap_opt_t *opt) {

    // Shortes options have their length encoded onto lower 4 bits of the 1st byte
    uint16_t length = *opt & 0x0f;

    // Inspect "Option Delta" field to check if delta notation is extended
    switch (*opt++ & 0xf0) {
    case 0xf0:
        coap_log(LOG_DEBUG, "coap_opt_length: illegal option delta\n");
        return 0;
    // If delta = 14, it's notation will be extended by 2 bytes
    // (i.e potential length's extension will start at 3rd byte)
    case 0xe0:
        opt += 2;
        break;
    // If delta = 13, it's notation will be extended by 1 byte
    // (i.e potential length's extension will start at 2nd byte)
    case 0xd0:
        opt += 1;
        break;
    }

    // Inspect "Option Length" field to check if length notation is extended
    switch (length) {
        case 0x0f:
            coap_log(LOG_DEBUG, "illegal option length\n");
            return 0;
        // If length notation is extended by 2 bytes; these bytes are equal to option's
        // value length substracted by 269
        case 0x0e:
            length = (opt[0] << 8) + opt[1] + 269;
            break;
        // If length notation is extended by 1 bytes; this bytes is equal to option's
        // value length substracted by 13 (i.e 0x0d)
        case 0x0d:
            length += opt[0];
            break;
    }

    return length;
}


const uint8_t *coap_opt_value(const coap_opt_t *opt){

    // Default offset of value in the option's field is 1 byte
    size_t data_offsset = 1;

    // Inspect "Option Delta" part of the option's header
    switch (*opt & 0xf0) {
        case 0xf0:
            coap_log(LOG_DEBUG, "illegal option delta\n");
            return 0;
        // If delta = 14, it's notation will be extended by 2 bytes
        case 0xe0:
            data_offsset += 2;
            break;
        // If delta = 13, it's notation will be extended by 1 byte
        case 0xd0:
            data_offsset += 1;
            break;
    }

    // Inspect "Option Length" part of the option's ehader
    switch (*opt & 0x0f) {
        case 0x0f:
            coap_log(LOG_DEBUG, "illegal option length\n");
            return 0;
        // If length = 14, it's notation will be extended by 2 bytes
        case 0x0e:
            data_offsset += 2;
            break;
        // If length = 13, it's notation will be extended by 1 byte
        case 0x0d:
            data_offsset += 1;
            break;

    }

    return (const uint8_t *)opt + data_offsset;
}


size_t coap_opt_size(const coap_opt_t *opt) {

    /**
     * @note: Here, we assume that @p opt is encoded correctly and
     *   pass a (technically) unlimited size of the option's buffer
     *  to the @f coap_opt_parse function.
     */

    coap_option_t option;
    return coap_opt_parse(opt, (size_t) UINT_MAX, &option);
}


size_t coap_opt_setheader(
    coap_opt_t *opt,
    size_t maxlen,
    uint16_t delta, 
    size_t length
) {
    assert(opt);
    if (maxlen == 0)
        return 0;

    // Number of bytes from @p opt to the first byte of option's value
    size_t data_offset = 0;

    /* ---------------------- Delta encoding ---------------------- */

    // No extesion
    if (delta < 13)
        opt[0] = (coap_opt_t) (delta << 4);
    // One-byte extension
    else if (delta < 269) {

        // Verify buffer's length
        if (maxlen < 2) {
            coap_log(LOG_DEBUG, "insufficient space to encode option delta %d\n", delta);
            return 0;
        }
        opt[0] = 0xd0;
        opt[++data_offset] = (coap_opt_t)(delta - 13);
    } 
    // Two-bytes extension
    else {

        // Verify buffer's length
        if (maxlen < 3) {
            coap_log(LOG_DEBUG, "insufficient space to encode option delta %d\n", delta);
            return 0;
        }

        opt[0] = 0xe0;
        opt[++data_offset] = ((delta - 269) >> 8) & 0xff;
        opt[++data_offset] = (delta - 269) & 0xff;
    }


    /* --------------------- Length encoding ---------------------- */
    
    // No extesion
    if (length < 13)
        opt[0] |= length & 0x0f;
    // One-byte extension
    else if (length < 269) {
        
        // Verify buffer's length
        if (maxlen < data_offset + 2) {
            coap_log(LOG_DEBUG, "insufficient space to encode option length %zu\n", length);
            return 0;
        }

        opt[0] |= 0x0d;
        opt[++data_offset] = (coap_opt_t)(length - 13);
    } 
    // Two-bytes extension
    else {

        // Verify buffer's length
        if (maxlen < data_offset + 3) {
            coap_log(LOG_DEBUG, "insufficient space to encode option delta %d\n", delta);
            return 0;
        }

        opt[0] |= 0x0e;
        opt[++data_offset] = ((length - 269) >> 8) & 0xff;
        opt[++data_offset] = (length - 269) & 0xff;
    }

    return data_offset + 1;
}


size_t coap_opt_encode_size(
    uint16_t delta, 
    size_t length
) {
    // Option consist of at least 1-byte header
    size_t size = 1;

    // Option's code encoding can be extended by 1 or 2 bytes
    if (delta >= 13) {
        if (delta < 269)
            size += 1;
        else
            size += 2;
    }

    // Option's length encoding can be extended by 1 or 2 bytes
    if (length >= 13) {
        if (length < 269)
            size += 1;
        else
            size += 2;
    }

    return size + length;
}


size_t coap_opt_encode(
    coap_opt_t *opt, 
    size_t maxlen, 
    uint16_t delta,
    size_t length,
    const uint8_t *val    
) {
    // Compute required size for storing the option and check if it fits the buffer
    size_t size = coap_opt_encode_size(delta, length);
    if (size > maxlen) {
        coap_log(LOG_DEBUG, "coap_opt_encode: option's buffer too small\n");
        return 0;
    }

    // Encode the header
    size_t opt_offset = coap_opt_setheader(opt, maxlen, delta, length);
    if (opt_offset == 0) {
        coap_log(LOG_DEBUG, "coap_opt_encode: cannot set option header\n");
        return 0;
    }

    // Copy option's value, if given
    if (val){
        opt += opt_offset;
        memcpy(opt, val, length);
    }

    return size;
}


/**
 * @param type:
 *    option's type (i.e. code)
 * @returns:
 *    true if @p type denotes an option type larger than 255 bytes
 *    false otherwise
 */
COAP_STATIC_INLINE int
is_long_option(uint16_t type) { return type > 255; }


/**
 * @brief: Applies @p filter_op on @p filter with respect to @p type. The following
 *   operations are defined:
 *
 *   FILTER_SET: Store @p type into an empty slot in @p filter. Returns
 *   @c 1 on success, or @c 0 if no spare slot was available.
 *  
 *   FILTER_CLEAR: Remove @p type from filter if it exists.
 *  
 *   FILTER_GET: Search for @p type in @p filter. Returns @c 1 if found,
 *   or @c 0 if not found.
 *
 * @param filter:
 *    the filter object
 * @param type:
 *    the option type to set, get or clear in @p filter
 * @param filter_op:
 *    the operation to apply to @p filter and @p type
 *
 * @returns
 *    1 on success
 *    0 when FILTER_GET yields no hit or no free slot is available to store @p type 
 *    with FILTER_SET
 */
static int coap_option_filter_op(
    coap_opt_filter_t filter,
    uint16_t type,
    enum filter_op_t filter_op
) {
    size_t lindex = 0;
    opt_filter *of = (opt_filter *)filter;
    uint16_t nr, mask = 0;

    // If @p type irepresents a long option ...
    if (is_long_option(type)) {

        
        mask = LONG_MASK;

        for (nr = 1; lindex < COAP_OPT_FILTER_LONG; nr <<= 1, lindex++) {

        if (((of->mask & nr) > 0) && (of->long_opts[lindex] == type)) {
            if (filter_op == FILTER_CLEAR) {
            of->mask &= ~nr;
            }

            return 1;
        }
        }
    }
    // Else, if @p type irepresents a short option ...
    else {
        mask = SHORT_MASK;

        for (nr = 1 << COAP_OPT_FILTER_LONG; lindex < COAP_OPT_FILTER_SHORT;
            nr <<= 1, lindex++) {

        if (((of->mask & nr) > 0) && (of->short_opts[lindex] == (type & 0xff))) {
            if (filter_op == FILTER_CLEAR) {
            of->mask &= ~nr;
            }

            return 1;
        }
        }
    }

    /* type was not found, so there is nothing to do if op is CLEAR or GET */
    if ((filter_op == FILTER_CLEAR) || (filter_op == FILTER_GET)) {
        return 0;
    }

    /* handle FILTER_SET: */

    lindex = coap_fls(~of->mask & mask);
    if (!lindex) {
        return 0;
    }

    if (is_long_option(type)) {
        of->long_opts[lindex - 1] = type;
    } else {
        of->short_opts[lindex - COAP_OPT_FILTER_LONG - 1] = (uint8_t)type;
    }

    of->mask |= 1 << (lindex - 1);

    return 1;
}

int
coap_option_filter_set(coap_opt_filter_t filter, uint16_t type) {
  return coap_option_filter_op(filter, type, FILTER_SET);
}

int
coap_option_filter_unset(coap_opt_filter_t filter, uint16_t type) {
  return coap_option_filter_op(filter, type, FILTER_CLEAR);
}

int
coap_option_filter_get(coap_opt_filter_t filter, uint16_t type) {
  /* Ugly cast to make the const go away (FILTER_GET wont change filter
   * but as _set and _unset do, the function does not take a const). */
  return coap_option_filter_op((uint16_t *)filter, type, FILTER_GET);
}

coap_optlist_t *
coap_new_optlist(uint16_t number,
                          size_t length,
                          const uint8_t *data
) {
  coap_optlist_t *node;

  node = (coap_optlist_t*) coap_malloc(sizeof(coap_optlist_t) + length);

  if (node) {
    memset(node, 0, (sizeof(coap_optlist_t) + length));
    node->number = number;
    node->length = length;
    node->data = (uint8_t *)&node[1];
    memcpy(node->data, data, length);
  } else {
    coap_log(LOG_WARNING, "coap_new_optlist: malloc failure\n");
  }

  return node;
}

static int
order_opts(void *a, void *b) {
  coap_optlist_t *o1 = (coap_optlist_t *)a;
  coap_optlist_t *o2 = (coap_optlist_t *)b;

  if (!a || !b)
    return a < b ? -1 : 1;

  return (int)(o1->number - o2->number);
}

int
coap_add_optlist_pdu(coap_pdu_t *pdu, coap_optlist_t** options) {
  coap_optlist_t *opt;

  if (options && *options) {
    /* sort options for delta encoding */
    LL_SORT((*options), order_opts);

    LL_FOREACH((*options), opt) {
      coap_add_option(pdu, opt->number, opt->length, opt->data);
    }
    return 1;
  }
  return 0;
}

int
coap_insert_optlist(coap_optlist_t **head, coap_optlist_t *node) {
  if (!node) {
    coap_log(LOG_DEBUG, "optlist not provided\n");
  } else {
    /* must append at the list end to avoid re-ordering of
     * options during sort */
    LL_APPEND((*head), node);
  }

  return node != NULL;
}

static int
coap_internal_delete(coap_optlist_t *node) {
  if (node) {
    coap_free(node);
  }
  return 1;
}

void
coap_delete_optlist(coap_optlist_t *queue) {
  coap_optlist_t *elt, *tmp;

  if (!queue)
    return;

  LL_FOREACH_SAFE(queue, elt, tmp) {
    coap_internal_delete(elt);
  }
}
