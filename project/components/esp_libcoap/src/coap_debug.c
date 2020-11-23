/* debug.c -- debug utilities
 *
 * Copyright (C) 2010--2012,2014--2019 Olaf Bergmann <bergmann@tzi.org> and others
 *
 * This file is part of the CoAP library libcoap. Please see
 * README for terms of use.
 */

#include "coap_config.h"

# include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <time.h>

#include "libcoap.h"
#include "block.h"
#include "coap_debug.h"
#include "encode.h"
#include "net.h"

/* -------------------------------------------- [Static symbols] ---------------------------------------------- */

// Current maximum log level
static coap_log_t maxlog = LOG_WARNING;

 // Controls printing PDUs with fprintf
static int use_fprintf_for_show_pdu = 1;

// String names of the @t coap_log_t levels
static const char *loglevels[] = {
  "EMRG", "ALRT", "CRIT", "ERR ", "WARN", "NOTE", "INFO", "DEBG"
};

/* ----------------------------------------------- [Functions] ------------------------------------------------ */

const char *coap_package_name(void){
  return PACKAGE_NAME;
}


const char *coap_package_version(void){
  return PACKAGE_STRING;
}


void coap_set_show_pdu_output(int use_fprintf){
  use_fprintf_for_show_pdu = use_fprintf;
}


coap_log_t coap_get_log_level(void){
  return maxlog;
}


void coap_set_log_level(coap_log_t level){
  maxlog = level;
}


COAP_STATIC_INLINE size_t
print_timestamp(char *s, size_t len, coap_tick_t t){
  struct tm *tmp;
  time_t now = coap_ticks_to_rt(t);
  tmp = localtime(&now);
  return strftime(s, len, "%b %d %H:%M:%S", tmp);
}


static size_t print_readable(
    const uint8_t *data,
    size_t len,
    unsigned char *result,
    size_t buflen,
    int encode_always
){
  const uint8_t hex[] = "0123456789ABCDEF";
  size_t cnt = 0;
  assert(data || len == 0);

  if (buflen == 0) { /* there is nothing we can do here but return */
    return 0;
  }

  while (len) {
    if (!encode_always && isprint(*data)) {
      if (cnt+1 < buflen) { /* keep one byte for terminating zero */
      *result++ = *data;
      ++cnt;
      } else {
        break;
      }
    } else {
      if (cnt+4 < buflen) { /* keep one byte for terminating zero */
        *result++ = '\\';
        *result++ = 'x';
        *result++ = hex[(*data & 0xf0) >> 4];
        *result++ = hex[*data & 0x0f];
        cnt += 4;
      } else
        break;
    }

    ++data; --len;
  }

  *result = '\0'; /* add a terminating zero */
  return cnt;
}

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

size_t
coap_print_addr(const struct coap_address_t *addr, unsigned char *buf, size_t len) {

  const void *addrptr = NULL;
  in_port_t port;
  unsigned char *p = buf;

  switch (addr->addr.sa.sa_family) {
  case AF_INET:
    addrptr = &addr->addr.sin.sin_addr;
    port = ntohs(addr->addr.sin.sin_port);
    break;
  case AF_INET6:
    if (len < 7) /* do not proceed if buffer is even too short for [::]:0 */
      return 0;

    *p++ = '[';

    addrptr = &addr->addr.sin6.sin6_addr;
    port = ntohs(addr->addr.sin6.sin6_port);

    break;
  default:
    memcpy(buf, "(unknown address type)", min(22, len));
    return min(22, len);
  }

  /* Cast needed for Windows, since it doesn't have the correct API signature. */
  if (inet_ntop(addr->addr.sa.sa_family, addrptr, (char *)p, len) == 0) {
    perror("coap_print_addr");
    return 0;
  }

  p += strnlen((char *)p, len);

  if (addr->addr.sa.sa_family == AF_INET6) {
    if (p < buf + len) {
      *p++ = ']';
    } else
      return 0;
  }

  p += snprintf((char *)p, buf + len - p + 1, ":%d", port);

  return buf + len - p;
}

/** Returns a textual description of the message type @p t. */
static const char *msg_type_string(uint16_t t){
  static const char *types[] = { "CON", "NON", "ACK", "RST", "???" };
  return types[min(t, sizeof(types)/sizeof(char *) - 1)];
}

/** Returns a textual description of the method or response code. */
static const char *msg_code_string(uint16_t c){
  static const char *methods[] = { "0.00", "GET", "POST", "PUT", "DELETE",
                                   "FETCH", "PATCH", "iPATCH" };
  static const char *signals[] = { "7.00", "CSM", "Ping", "Pong", "Release",
                                   "Abort" };
  static char buf[5];

  if (c < sizeof(methods)/sizeof(const char *)) {
    return methods[c];
  } else if (c >= 224 && c - 224 < (int)(sizeof(signals)/sizeof(const char *))) {
    return signals[c-224];
  } else {
    snprintf(buf, sizeof(buf), "%u.%02u", c >> 5, c & 0x1f);
    return buf;
  }
}

/** Returns a textual description of the option name. */
static const char *
msg_option_string(uint8_t code, uint16_t option_type) {
  struct option_desc_t {
    uint16_t type;
    const char *name;
  };

  static struct option_desc_t options[] = {
    { COAP_OPTION_IF_MATCH, "If-Match" },
    { COAP_OPTION_URI_HOST, "Uri-Host" },
    { COAP_OPTION_ETAG, "ETag" },
    { COAP_OPTION_IF_NONE_MATCH, "If-None-Match" },
    { COAP_OPTION_OBSERVE, "Observe" },
    { COAP_OPTION_URI_PORT, "Uri-Port" },
    { COAP_OPTION_LOCATION_PATH, "Location-Path" },
    { COAP_OPTION_URI_PATH, "Uri-Path" },
    { COAP_OPTION_CONTENT_FORMAT, "Content-Format" },
    { COAP_OPTION_MAXAGE, "Max-Age" },
    { COAP_OPTION_URI_QUERY, "Uri-Query" },
    { COAP_OPTION_ACCEPT, "Accept" },
    { COAP_OPTION_LOCATION_QUERY, "Location-Query" },
    { COAP_OPTION_BLOCK2, "Block2" },
    { COAP_OPTION_BLOCK1, "Block1" },
    { COAP_OPTION_PROXY_URI, "Proxy-Uri" },
    { COAP_OPTION_PROXY_SCHEME, "Proxy-Scheme" },
    { COAP_OPTION_SIZE1, "Size1" },
    { COAP_OPTION_SIZE2, "Size2" },
    { COAP_OPTION_NORESPONSE, "No-Response" }
  };

  static struct option_desc_t options_csm[] = {
    { COAP_SIGNALING_OPTION_MAX_MESSAGE_SIZE, "Max-Message-Size" },
    { COAP_SIGNALING_OPTION_BLOCK_WISE_TRANSFER, "Block-wise-Transfer" }
  };

  static struct option_desc_t options_pingpong[] = {
    { COAP_SIGNALING_OPTION_CUSTODY, "Custody" }
  };

  static struct option_desc_t options_release[] = {
    { COAP_SIGNALING_OPTION_ALTERNATIVE_ADDRESS, "Alternative-Address" },
    { COAP_SIGNALING_OPTION_HOLD_OFF, "Hold-Off" }
  };

  static struct option_desc_t options_abort[] = {
    { COAP_SIGNALING_OPTION_BAD_CSM_OPTION, "Bad-CSM-Option" }
  };

  static char buf[6];
  size_t i;

  if (code == COAP_SIGNALING_CSM) {
    for (i = 0; i < sizeof(options_csm)/sizeof(struct option_desc_t); i++) {
      if (option_type == options_csm[i].type) {
        return options_csm[i].name;
      }
    }
  } else if (code == COAP_SIGNALING_PING || code == COAP_SIGNALING_PONG) {
    for (i = 0; i < sizeof(options_pingpong)/sizeof(struct option_desc_t); i++) {
      if (option_type == options_pingpong[i].type) {
        return options_pingpong[i].name;
      }
    }
  } else if (code == COAP_SIGNALING_RELEASE) {
    for (i = 0; i < sizeof(options_release)/sizeof(struct option_desc_t); i++) {
      if (option_type == options_release[i].type) {
        return options_release[i].name;
      }
    }
  } else if (code == COAP_SIGNALING_ABORT) {
    for (i = 0; i < sizeof(options_abort)/sizeof(struct option_desc_t); i++) {
      if (option_type == options_abort[i].type) {
        return options_abort[i].name;
      }
    }
  } else {
    /* search option_type in list of known options */
    for (i = 0; i < sizeof(options)/sizeof(struct option_desc_t); i++) {
      if (option_type == options[i].type) {
        return options[i].name;
      }
    }
  }
  /* unknown option type, just print to buf */
  snprintf(buf, sizeof(buf), "%u", option_type);
  return buf;
}

static unsigned int
print_content_format(unsigned int format_type,
                     unsigned char *result, unsigned int buflen) {
  struct desc_t {
    unsigned int type;
    const char *name;
  };

  static struct desc_t formats[] = {
    { COAP_MEDIATYPE_TEXT_PLAIN, "text/plain" },
    { COAP_MEDIATYPE_APPLICATION_LINK_FORMAT, "application/link-format" },
    { COAP_MEDIATYPE_APPLICATION_XML, "application/xml" },
    { COAP_MEDIATYPE_APPLICATION_OCTET_STREAM, "application/octet-stream" },
    { COAP_MEDIATYPE_APPLICATION_EXI, "application/exi" },
    { COAP_MEDIATYPE_APPLICATION_JSON, "application/json" },
    { COAP_MEDIATYPE_APPLICATION_CBOR, "application/cbor" },
    { COAP_MEDIATYPE_APPLICATION_COSE_SIGN, "application/cose; cose-type=\"cose-sign\"" },
    { COAP_MEDIATYPE_APPLICATION_COSE_SIGN1, "application/cose; cose-type=\"cose-sign1\"" },
    { COAP_MEDIATYPE_APPLICATION_COSE_ENCRYPT, "application/cose; cose-type=\"cose-encrypt\"" },
    { COAP_MEDIATYPE_APPLICATION_COSE_ENCRYPT0, "application/cose; cose-type=\"cose-encrypt0\"" },
    { COAP_MEDIATYPE_APPLICATION_COSE_MAC, "application/cose; cose-type=\"cose-mac\"" },
    { COAP_MEDIATYPE_APPLICATION_COSE_MAC0, "application/cose; cose-type=\"cose-mac0\"" },
    { COAP_MEDIATYPE_APPLICATION_COSE_KEY, "application/cose-key" },
    { COAP_MEDIATYPE_APPLICATION_COSE_KEY_SET, "application/cose-key-set" },
    { COAP_MEDIATYPE_APPLICATION_SENML_JSON, "application/senml+json" },
    { COAP_MEDIATYPE_APPLICATION_SENSML_JSON, "application/sensml+json" },
    { COAP_MEDIATYPE_APPLICATION_SENML_CBOR, "application/senml+cbor" },
    { COAP_MEDIATYPE_APPLICATION_SENSML_CBOR, "application/sensml+cbor" },
    { COAP_MEDIATYPE_APPLICATION_SENML_EXI, "application/senml-exi" },
    { COAP_MEDIATYPE_APPLICATION_SENSML_EXI, "application/sensml-exi" },
    { COAP_MEDIATYPE_APPLICATION_SENML_XML, "application/senml+xml" },
    { COAP_MEDIATYPE_APPLICATION_SENSML_XML, "application/sensml+xml" },
    { 75, "application/dcaf+cbor" }
  };

  size_t i;

  /* search format_type in list of known content formats */
  for (i = 0; i < sizeof(formats)/sizeof(struct desc_t); i++) {
    if (format_type == formats[i].type) {
      return snprintf((char *)result, buflen, "%s", formats[i].name);
    }
  }

  /* unknown content format, just print numeric value to buf */
  return snprintf((char *)result, buflen, "%d", format_type);
}

/**
 * Returns 1 if the given @p content_format is either unknown or known
 * to carry binary data. The return value @c 0 hence indicates
 * printable data which is also assumed if @p content_format is @c 01.
 */
COAP_STATIC_INLINE int
is_binary(int content_format) {
  return !(content_format == -1 ||
           content_format == COAP_MEDIATYPE_TEXT_PLAIN ||
           content_format == COAP_MEDIATYPE_APPLICATION_LINK_FORMAT ||
           content_format == COAP_MEDIATYPE_APPLICATION_XML ||
           content_format == COAP_MEDIATYPE_APPLICATION_JSON);
}

#define COAP_DO_SHOW_OUTPUT_LINE           \
 do {                                      \
   if (use_fprintf_for_show_pdu) {         \
     fprintf(COAP_DEBUG_FD, "%s", outbuf); \
   }                                       \
   else {                                  \
     coap_log(level, "%s", outbuf);        \
   }                                       \
 } while (0)

void
coap_show_pdu(coap_log_t level, const coap_pdu_t *pdu) {
  unsigned char buf[1024]; /* need some space for output creation */
  size_t buf_len = 0; /* takes the number of bytes written to buf */
  int encode = 0, have_options = 0, i;
  coap_opt_iterator_t opt_iter;
  coap_opt_t *option;
  int content_format = -1;
  size_t data_len;
  unsigned char *data;
  char outbuf[COAP_DEBUG_BUF_SIZE];
  int outbuflen = 0;

  /* Save time if not needed */
  if (level > coap_get_log_level())
    return;

  snprintf(outbuf, sizeof(outbuf), "v:%d t:%s c:%s i:%04x {",
          COAP_DEFAULT_VERSION, msg_type_string(pdu->type),
          msg_code_string(pdu->code), pdu->tid);

  for (i = 0; i < pdu->token_length; i++) {
    outbuflen = strlen(outbuf);
    snprintf(&outbuf[outbuflen], sizeof(outbuf)-outbuflen,
              "%02x", pdu->token[i]);
  }
  outbuflen = strlen(outbuf);
  snprintf(&outbuf[outbuflen], sizeof(outbuf)-outbuflen,  "}");

  /* show options, if any */
  coap_option_iterator_init(pdu, &opt_iter, COAP_OPT_ALL);

  outbuflen = strlen(outbuf);
  snprintf(&outbuf[outbuflen], sizeof(outbuf)-outbuflen,  " [");
  while ((option = coap_option_next(&opt_iter))) {
    if (!have_options) {
      have_options = 1;
    } else {
      outbuflen = strlen(outbuf);
      snprintf(&outbuf[outbuflen], sizeof(outbuf)-outbuflen,  ",");
    }

    if (pdu->code == COAP_SIGNALING_CSM) switch(opt_iter.type) {
    case COAP_SIGNALING_OPTION_MAX_MESSAGE_SIZE:
      buf_len = snprintf((char *)buf, sizeof(buf), "%u",
                         coap_decode_var_bytes(coap_opt_value(option),
                                               coap_opt_length(option)));
      break;
    default:
      buf_len = 0;
      break;
    } else if (pdu->code == COAP_SIGNALING_PING
            || pdu->code == COAP_SIGNALING_PONG) {
      buf_len = 0;
    } else if (pdu->code == COAP_SIGNALING_RELEASE) switch(opt_iter.type) {
    case COAP_SIGNALING_OPTION_ALTERNATIVE_ADDRESS:
      buf_len = print_readable(coap_opt_value(option),
                               coap_opt_length(option),
                               buf, sizeof(buf), 0);
      break;
    case COAP_SIGNALING_OPTION_HOLD_OFF:
      buf_len = snprintf((char *)buf, sizeof(buf), "%u",
                         coap_decode_var_bytes(coap_opt_value(option),
                                               coap_opt_length(option)));
      break;
    default:
      buf_len = 0;
      break;
    } else if (pdu->code == COAP_SIGNALING_ABORT) switch(opt_iter.type) {
    case COAP_SIGNALING_OPTION_BAD_CSM_OPTION:
      buf_len = snprintf((char *)buf, sizeof(buf), "%u",
                         coap_decode_var_bytes(coap_opt_value(option),
                                               coap_opt_length(option)));
      break;
    default:
      buf_len = 0;
      break;
    } else switch (opt_iter.type) {
    case COAP_OPTION_CONTENT_FORMAT:
      content_format = (int)coap_decode_var_bytes(coap_opt_value(option),
                                                  coap_opt_length(option));

      buf_len = print_content_format(content_format, buf, sizeof(buf));
      break;

    case COAP_OPTION_BLOCK1:
    case COAP_OPTION_BLOCK2:
      /* split block option into number/more/size where more is the
       * letter M if set, the _ otherwise */
      buf_len = snprintf((char *)buf, sizeof(buf), "%u/%c/%u",
                         coap_opt_block_num(option), /* block number */
                         COAP_OPT_BLOCK_MORE(option) ? 'M' : '_', /* M bit */
                         (1 << (COAP_OPT_BLOCK_SZX(option) + 4))); /* block size */

      break;

    case COAP_OPTION_URI_PORT:
    case COAP_OPTION_MAXAGE:
    case COAP_OPTION_OBSERVE:
    case COAP_OPTION_SIZE1:
    case COAP_OPTION_SIZE2:
      /* show values as unsigned decimal value */
      buf_len = snprintf((char *)buf, sizeof(buf), "%u",
                         coap_decode_var_bytes(coap_opt_value(option),
                                               coap_opt_length(option)));
      break;

    default:
      /* generic output function for all other option types */
      if (opt_iter.type == COAP_OPTION_URI_PATH ||
          opt_iter.type == COAP_OPTION_PROXY_URI ||
          opt_iter.type == COAP_OPTION_URI_HOST ||
          opt_iter.type == COAP_OPTION_LOCATION_PATH ||
          opt_iter.type == COAP_OPTION_LOCATION_QUERY ||
          opt_iter.type == COAP_OPTION_URI_QUERY) {
        encode = 0;
      } else {
        encode = 1;
      }

      buf_len = print_readable(coap_opt_value(option),
                               coap_opt_length(option),
                               buf, sizeof(buf), encode);
    }

    outbuflen = strlen(outbuf);
    snprintf(&outbuf[outbuflen], sizeof(outbuf)-outbuflen,
              " %s:%.*s", msg_option_string(pdu->code, opt_iter.type),
              (int)buf_len, buf);
  }

  outbuflen = strlen(outbuf);
  snprintf(&outbuf[outbuflen], sizeof(outbuf)-outbuflen,  " ]");

  if (coap_get_data(pdu, &data_len, &data)) {

    outbuflen = strlen(outbuf);
    snprintf(&outbuf[outbuflen], sizeof(outbuf)-outbuflen,  " :: ");

    if (is_binary(content_format)) {
      int keep_data_len = data_len;
      uint8_t *keep_data = data;

      outbuflen = strlen(outbuf);
      snprintf(&outbuf[outbuflen], sizeof(outbuf)-outbuflen,
               "binary data length %zu\n", data_len);
      COAP_DO_SHOW_OUTPUT_LINE;
      /*
       * Output hex dump of binary data as a continuous entry
       */
      outbuf[0] = '\000';
      snprintf(outbuf, sizeof(outbuf),  "<<");
      while (data_len--) {
        outbuflen = strlen(outbuf);
        snprintf(&outbuf[outbuflen], sizeof(outbuf)-outbuflen,
                 "%02x", *data++);
      }
      outbuflen = strlen(outbuf);
      snprintf(&outbuf[outbuflen], sizeof(outbuf)-outbuflen,  ">>");
      data_len = keep_data_len;
      data = keep_data;
      outbuflen = strlen(outbuf);
      snprintf(&outbuf[outbuflen], sizeof(outbuf)-outbuflen,  "\n");
      COAP_DO_SHOW_OUTPUT_LINE;
      /*
       * Output ascii readable (if possible), immediately under the
       * hex value of the character output above to help binary debugging
       */
      outbuf[0] = '\000';
      snprintf(outbuf, sizeof(outbuf),  "<<");
      while (data_len--) {
        outbuflen = strlen(outbuf);
        snprintf(&outbuf[outbuflen], sizeof(outbuf)-outbuflen,
                 "%c ", isprint (*data) ? *data : '.');
        data++;
      }
      outbuflen = strlen(outbuf);
      snprintf(&outbuf[outbuflen], sizeof(outbuf)-outbuflen,  ">>");
    } else {
      if (print_readable(data, data_len, buf, sizeof(buf), 0)) {
        outbuflen = strlen(outbuf);
        snprintf(&outbuf[outbuflen], sizeof(outbuf)-outbuflen,  "'%s'", buf);
      }
    }
  }

  outbuflen = strlen(outbuf);
  snprintf(&outbuf[outbuflen], sizeof(outbuf)-outbuflen,  "\n");
  COAP_DO_SHOW_OUTPUT_LINE;
}

static coap_log_handler_t log_handler = NULL;

void coap_set_log_handler(coap_log_handler_t handler) {
  log_handler = handler;
}

void
coap_log_impl(coap_log_t level, const char *format, ...) {

  if (maxlog < level)
    return;

  if (log_handler) {
    char message[8 + 1024 * 2]; /* O/H + Max packet payload size * 2 */
    va_list ap;
    va_start(ap, format);
    vsnprintf( message, sizeof(message), format, ap);
    va_end(ap);
    log_handler(level, message);
  } else {
    char timebuf[32];
    coap_tick_t now;
    va_list ap;
    FILE *log_fd;

    log_fd = level <= LOG_CRIT ? COAP_ERR_FD : COAP_DEBUG_FD;

    coap_ticks(&now);
    if (print_timestamp(timebuf,sizeof(timebuf), now))
      fprintf(log_fd, "%s ", timebuf);

    if (level <= LOG_DEBUG)
      fprintf(log_fd, "%s ", loglevels[level]);

    va_start(ap, format);
    vfprintf(log_fd, format, ap);
    va_end(ap);
    fflush(log_fd);
  }
}

static struct packet_num_interval {
  int start;
  int end;
} packet_loss_intervals[10];
static int num_packet_loss_intervals = 0;
static int packet_loss_level = 0;
static int send_packet_count = 0;

int coap_debug_set_packet_loss(const char *loss_level) {
  const char *p = loss_level;
  char *end = NULL;
  int n = (int)strtol(p, &end, 10), i = 0;
  if (end == p || n < 0)
    return 0;
  if (*end == '%') {
    if (n > 100)
      n = 100;
    packet_loss_level = n * 65536 / 100;
    coap_log(LOG_DEBUG, "packet loss level set to %d%%\n", n);
  } else {
    if (n <= 0)
      return 0;
    while (i < 10) {
      packet_loss_intervals[i].start = n;
      if (*end == '-') {
        p = end + 1;
        n = (int)strtol(p, &end, 10);
        if (end == p || n <= 0)
          return 0;
      }
      packet_loss_intervals[i++].end = n;
      if (*end == 0)
        break;
      if (*end != ',')
        return 0;
      p = end + 1;
      n = (int)strtol(p, &end, 10);
      if (end == p || n <= 0)
        return 0;
    }
    if (i == 10)
      return 0;
    num_packet_loss_intervals = i;
  }
  send_packet_count = 0;
  return 1;
}

int coap_debug_send_packet(void) {
  ++send_packet_count;
  if (num_packet_loss_intervals > 0) {
    int i;
    for (i = 0; i < num_packet_loss_intervals; i++) {
      if (send_packet_count >= packet_loss_intervals[i].start
        && send_packet_count <= packet_loss_intervals[i].end)
        return 0;
    }
  }
  if ( packet_loss_level > 0 ) {
    uint16_t r = 0;
    prng( (uint8_t*)&r, 2 );
    if ( r < packet_loss_level )
      return 0;
  }
  return 1;
}
