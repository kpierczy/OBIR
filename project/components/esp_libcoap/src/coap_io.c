/* coap_io.c -- Default network I/O functions for libcoap
 *
 * Copyright (C) 2012,2014,2016-2019 Olaf Bergmann <bergmann@tzi.org> and others
 *
 * This file is part of the CoAP library libcoap. Please see
 * README for terms of use.
 */

#include "coap_config.h"

#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include "libcoap.h"
#include "coap_debug.h"
#include "mem.h"
#include "net.h"
#include "coap_io.h"
#include "pdu.h"
#include "utlist.h"
#include "resource.h"

#define OPTVAL_T(t)         (t)
#define OPTVAL_GT(t)        (t)

// Define generic PKTINFO for IPv4
#define GEN_IP_PKTINFO IP_PKTINFO
// Define generic KTINFO for IPv6
#define GEN_IPV6_PKTINFO IPV6_PKTINFO


void coap_free_endpoint(coap_endpoint_t *ep);
static const char *coap_socket_format_errno( int error );


struct coap_endpoint_t *coap_malloc_endpoint(void){
    return (struct coap_endpoint_t *)coap_malloc_type(COAP_ENDPOINT, sizeof(struct coap_endpoint_t));
}

void coap_mfree_endpoint(struct coap_endpoint_t *ep){
  coap_session_mfree(&ep->hello);
  coap_free_type(COAP_ENDPOINT, ep);
}

int coap_socket_bind_udp(
    coap_socket_t *sock,
    const coap_address_t *listen_addr,
    coap_address_t *bound_addr
){

    // Define options for @f setsockopt()
    int on = 1, off = 0;

    // Create system socket
    sock->fd = socket(listen_addr->addr.sa.sa_family, SOCK_DGRAM, 0);
    if (sock->fd == COAP_INVALID_SOCKET) {
        coap_log(LOG_WARNING, "coap_socket_bind_udp: socket: %s\n", coap_socket_strerror());
        goto error;
    }

    // Set socket to non-blocking mode
    if (ioctl(sock->fd, FIONBIO, &on) == COAP_SOCKET_ERROR) {
        coap_log(LOG_WARNING, "coap_socket_bind_udp: ioctl FIONBIO: %s\n", coap_socket_strerror());
    }

    // Set socket to have reusable address
    if (setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR, OPTVAL_T(&on), sizeof(on)) == COAP_SOCKET_ERROR){
        coap_log(LOG_WARNING, "coap_socket_bind_udp: setsockopt SO_REUSEADDR: %s\n",
            coap_socket_strerror());
    }

    // Set IP-gen-dependent options
    switch (listen_addr->addr.sa.sa_family){
        case AF_INET: // IPv4
            
            // Set IP_PKTINFO on the socket
            if (setsockopt(sock->fd, IPPROTO_IP, IP_PKTINFO, OPTVAL_T(&on), sizeof(on)) == COAP_SOCKET_ERROR){
                coap_log(LOG_ALERT, "coap_socket_bind_udp: setsockopt IP_PKTINFO: %s\n",
                    coap_socket_strerror());
            }
            break;

        case AF_INET6: // IPv6

            // Configure the socket as dual-stacked
            if (setsockopt(sock->fd, IPPROTO_IPV6, IPV6_V6ONLY, OPTVAL_T(&off), sizeof(off)) == COAP_SOCKET_ERROR){
                coap_log(LOG_ALERT, "coap_socket_bind_udp: setsockopt IPV6_V6ONLY: %s\n",
                    coap_socket_strerror());
            }

            // Set IP_PKTINFO on the socket
            if (setsockopt(sock->fd, IPPROTO_IPV6, IPV6_PKTINFO, OPTVAL_T(&on), sizeof(on)) == COAP_SOCKET_ERROR){
                coap_log(LOG_ALERT, "coap_socket_bind_udp: setsockopt IPV6_PKTINFO: %s\n",
                    coap_socket_strerror());
            }
            // Set IP_PKTINFO on the socket
            setsockopt(sock->fd, IPPROTO_IP, IP_PKTINFO, OPTVAL_T(&on), sizeof(on));

            break;

        default: // Unknown protocol

            coap_log(LOG_ALERT, "coap_socket_bind_udp: unsupported sa_family\n");
            break;
    }

    // Bind socket to the @p listen_addr
    if (bind(sock->fd, &listen_addr->addr.sa, listen_addr->size) == COAP_SOCKET_ERROR) {
        coap_log(LOG_WARNING, "coap_socket_bind_udp: bind: %s\n", coap_socket_strerror());
        goto error;
    }

    // Get local address bound with the socket by the system
    bound_addr->size = (socklen_t)sizeof(*bound_addr);
    if (getsockname(sock->fd, &bound_addr->addr.sa, &bound_addr->size) < 0) {
        coap_log(LOG_WARNING,
                "coap_socket_bind_udp: getsockname: %s\n",
                coap_socket_strerror());
        goto error;
    }

    return 1;

    // On error, close socket
error:
    coap_socket_close(sock);
    return 0;
}


int
coap_socket_connect_tcp1(coap_socket_t *sock,
                         const coap_address_t *local_if,
                         const coap_address_t *server,
                         int default_port,
                         coap_address_t *local_addr,
                         coap_address_t *remote_addr) {
  int on = 1, off = 0;
#ifdef _WIN32
  u_long u_on = 1;
#endif
  coap_address_t connect_addr;
  coap_address_copy( &connect_addr, server );

  sock->flags &= ~COAP_SOCKET_CONNECTED;
  sock->fd = socket(server->addr.sa.sa_family, SOCK_STREAM, 0);

  if (sock->fd == COAP_INVALID_SOCKET) {
    coap_log(LOG_WARNING,
             "coap_socket_connect_tcp1: socket: %s\n",
             coap_socket_strerror());
    goto error;
  }

#ifdef _WIN32
  if (ioctlsocket(sock->fd, FIONBIO, &u_on) == COAP_SOCKET_ERROR) {
#else
  if (ioctl(sock->fd, FIONBIO, &on) == COAP_SOCKET_ERROR) {
#endif
    coap_log(LOG_WARNING,
             "coap_socket_connect_tcp1: ioctl FIONBIO: %s\n",
             coap_socket_strerror());
  }

  switch (server->addr.sa.sa_family) {
  case AF_INET:
    if (connect_addr.addr.sin.sin_port == 0)
      connect_addr.addr.sin.sin_port = htons(default_port);
    break;
  case AF_INET6:
    if (connect_addr.addr.sin6.sin6_port == 0)
      connect_addr.addr.sin6.sin6_port = htons(default_port);
    /* Configure the socket as dual-stacked */
    if (setsockopt(sock->fd, IPPROTO_IPV6, IPV6_V6ONLY, OPTVAL_T(&off), sizeof(off)) == COAP_SOCKET_ERROR)
      coap_log(LOG_WARNING,
               "coap_socket_connect_tcp1: setsockopt IPV6_V6ONLY: %s\n",
               coap_socket_strerror());
    break;
  default:
    coap_log(LOG_ALERT, "coap_socket_connect_tcp1: unsupported sa_family\n");
    break;
  }

  if (local_if && local_if->addr.sa.sa_family) {
    coap_address_copy(local_addr, local_if);
    if (setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR, OPTVAL_T(&on), sizeof(on)) == COAP_SOCKET_ERROR)
      coap_log(LOG_WARNING,
               "coap_socket_connect_tcp1: setsockopt SO_REUSEADDR: %s\n",
               coap_socket_strerror());
    if (bind(sock->fd, &local_if->addr.sa, local_if->size) == COAP_SOCKET_ERROR) {
      coap_log(LOG_WARNING, "coap_socket_connect_tcp1: bind: %s\n",
               coap_socket_strerror());
      goto error;
    }
  } else {
    local_addr->addr.sa.sa_family = server->addr.sa.sa_family;
  }

  if (connect(sock->fd, &connect_addr.addr.sa, connect_addr.size) == COAP_SOCKET_ERROR) {
#ifdef _WIN32
    if (WSAGetLastError() == WSAEWOULDBLOCK) {
#else
    if (errno == EINPROGRESS) {
#endif
      /*
       * COAP_SOCKET_CONNECTED needs to be set here as there will be reads/writes
       * by underlying TLS libraries during connect() and we do not want to
       * assert() in coap_read_session() or coap_write_session() when called by coap_read()
       */
      sock->flags |= COAP_SOCKET_WANT_CONNECT | COAP_SOCKET_CONNECTED;
      return 1;
    }
    coap_log(LOG_WARNING, "coap_socket_connect_tcp1: connect: %s\n",
             coap_socket_strerror());
    goto error;
  }

  if (getsockname(sock->fd, &local_addr->addr.sa, &local_addr->size) == COAP_SOCKET_ERROR) {
    coap_log(LOG_WARNING, "coap_socket_connect_tcp1: getsockname: %s\n",
             coap_socket_strerror());
  }

  if (getpeername(sock->fd, &remote_addr->addr.sa, &remote_addr->size) == COAP_SOCKET_ERROR) {
    coap_log(LOG_WARNING, "coap_socket_connect_tcp1: getpeername: %s\n",
             coap_socket_strerror());
  }

  sock->flags |= COAP_SOCKET_CONNECTED;
  return 1;

error:
  coap_socket_close(sock);
  return 0;
}

int
coap_socket_connect_tcp2(coap_socket_t *sock,
                         coap_address_t *local_addr,
                         coap_address_t *remote_addr) {
  int error = 0;
#ifdef _WIN32
  int optlen = (int)sizeof( error );
#else
  socklen_t optlen = (socklen_t)sizeof( error );
#endif

  sock->flags &= ~(COAP_SOCKET_WANT_CONNECT | COAP_SOCKET_CAN_CONNECT);

  if (getsockopt(sock->fd, SOL_SOCKET, SO_ERROR, OPTVAL_GT(&error),
    &optlen) == COAP_SOCKET_ERROR) {
    coap_log(LOG_WARNING, "coap_socket_finish_connect_tcp: getsockopt: %s\n",
      coap_socket_strerror());
  }

  if (error) {
    coap_log(LOG_WARNING,
             "coap_socket_finish_connect_tcp: connect failed: %s\n",
             coap_socket_format_errno(error));
    coap_socket_close(sock);
    return 0;
  }

  if (getsockname(sock->fd, &local_addr->addr.sa, &local_addr->size) == COAP_SOCKET_ERROR) {
    coap_log(LOG_WARNING, "coap_socket_connect_tcp: getsockname: %s\n",
             coap_socket_strerror());
  }

  if (getpeername(sock->fd, &remote_addr->addr.sa, &remote_addr->size) == COAP_SOCKET_ERROR) {
    coap_log(LOG_WARNING, "coap_socket_connect_tcp: getpeername: %s\n",
             coap_socket_strerror());
  }

  return 1;
}

int
coap_socket_bind_tcp(coap_socket_t *sock,
                     const coap_address_t *listen_addr,
                     coap_address_t *bound_addr) {
  int on = 1, off = 0;
#ifdef _WIN32
  u_long u_on = 1;
#endif

  sock->fd = socket(listen_addr->addr.sa.sa_family, SOCK_STREAM, 0);

  if (sock->fd == COAP_INVALID_SOCKET) {
    coap_log(LOG_WARNING, "coap_socket_bind_tcp: socket: %s\n",
             coap_socket_strerror());
    goto error;
  }

#ifdef _WIN32
  if (ioctlsocket(sock->fd, FIONBIO, &u_on) == COAP_SOCKET_ERROR) {
#else
  if (ioctl(sock->fd, FIONBIO, &on) == COAP_SOCKET_ERROR) {
#endif
    coap_log(LOG_WARNING, "coap_socket_bind_tcp: ioctl FIONBIO: %s\n",
                           coap_socket_strerror());
  }
  if (setsockopt (sock->fd, SOL_SOCKET, SO_KEEPALIVE, OPTVAL_T(&on),
                  sizeof (on)) == COAP_SOCKET_ERROR)
    coap_log(LOG_WARNING,
             "coap_socket_bind_tcp: setsockopt SO_KEEPALIVE: %s\n",
             coap_socket_strerror());

  if (setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR, OPTVAL_T(&on),
                 sizeof(on)) == COAP_SOCKET_ERROR)
    coap_log(LOG_WARNING,
             "coap_socket_bind_tcp: setsockopt SO_REUSEADDR: %s\n",
             coap_socket_strerror());

  switch (listen_addr->addr.sa.sa_family) {
  case AF_INET:
    break;
  case AF_INET6:
    /* Configure the socket as dual-stacked */
    if (setsockopt(sock->fd, IPPROTO_IPV6, IPV6_V6ONLY, OPTVAL_T(&off), sizeof(off)) == COAP_SOCKET_ERROR)
      coap_log(LOG_ALERT,
               "coap_socket_bind_tcp: setsockopt IPV6_V6ONLY: %s\n",
               coap_socket_strerror());
    break;
  default:
    coap_log(LOG_ALERT, "coap_socket_bind_tcp: unsupported sa_family\n");
  }

  if (bind(sock->fd, &listen_addr->addr.sa, listen_addr->size) == COAP_SOCKET_ERROR) {
    coap_log(LOG_ALERT, "coap_socket_bind_tcp: bind: %s\n",
             coap_socket_strerror());
    goto error;
  }

  bound_addr->size = (socklen_t)sizeof(*bound_addr);
  if (getsockname(sock->fd, &bound_addr->addr.sa, &bound_addr->size) < 0) {
    coap_log(LOG_WARNING, "coap_socket_bind_tcp: getsockname: %s\n",
             coap_socket_strerror());
    goto error;
  }

  if (listen(sock->fd, 5) == COAP_SOCKET_ERROR) {
    coap_log(LOG_ALERT, "coap_socket_bind_tcp: listen: %s\n",
             coap_socket_strerror());
    goto  error;
  }

  return 1;

error:
  coap_socket_close(sock);
  return 0;
}

int
coap_socket_accept_tcp(coap_socket_t *server,
                       coap_socket_t *new_client,
                       coap_address_t *local_addr,
                       coap_address_t *remote_addr) {
#ifdef _WIN32
  u_long u_on = 1;
#else
  int on = 1;
#endif

  server->flags &= ~COAP_SOCKET_CAN_ACCEPT;

  new_client->fd = accept(server->fd, &remote_addr->addr.sa,
                          &remote_addr->size);
  if (new_client->fd == COAP_INVALID_SOCKET) {
    coap_log(LOG_WARNING, "coap_socket_accept_tcp: accept: %s\n",
             coap_socket_strerror());
    return 0;
  }

  if (getsockname( new_client->fd, &local_addr->addr.sa, &local_addr->size) < 0)
    coap_log(LOG_WARNING, "coap_socket_accept_tcp: getsockname: %s\n",
             coap_socket_strerror());

  #ifdef _WIN32
  if (ioctlsocket(new_client->fd, FIONBIO, &u_on) == COAP_SOCKET_ERROR) {
#else
  if (ioctl(new_client->fd, FIONBIO, &on) == COAP_SOCKET_ERROR) {
#endif
    coap_log(LOG_WARNING, "coap_socket_accept_tcp: ioctl FIONBIO: %s\n",
             coap_socket_strerror());
  }

  return 1;
}

int
coap_socket_connect_udp(coap_socket_t *sock,
  const coap_address_t *local_if,
  const coap_address_t *server,
  int default_port,
  coap_address_t *local_addr,
  coap_address_t *remote_addr) {
  int on = 1, off = 0;
#ifdef _WIN32
  u_long u_on = 1;
#endif
  coap_address_t connect_addr;
  int is_mcast = coap_is_mcast(server);
  coap_address_copy(&connect_addr, server);

  sock->flags &= ~(COAP_SOCKET_CONNECTED | COAP_SOCKET_MULTICAST);
  sock->fd = socket(connect_addr.addr.sa.sa_family, SOCK_DGRAM, 0);

  if (sock->fd == COAP_INVALID_SOCKET) {
    coap_log(LOG_WARNING, "coap_socket_connect_udp: socket: %s\n",
             coap_socket_strerror());
    goto error;
  }

#ifdef _WIN32
  if (ioctlsocket(sock->fd, FIONBIO, &u_on) == COAP_SOCKET_ERROR) {
#else
  if (ioctl(sock->fd, FIONBIO, &on) == COAP_SOCKET_ERROR) {
#endif
    coap_log(LOG_WARNING, "coap_socket_connect_udp: ioctl FIONBIO: %s\n",
             coap_socket_strerror());
  }

  switch (connect_addr.addr.sa.sa_family) {
  case AF_INET:
    if (connect_addr.addr.sin.sin_port == 0)
      connect_addr.addr.sin.sin_port = htons(default_port);
    break;
  case AF_INET6:
    if (connect_addr.addr.sin6.sin6_port == 0)
      connect_addr.addr.sin6.sin6_port = htons(default_port);
    /* Configure the socket as dual-stacked */
    if (setsockopt(sock->fd, IPPROTO_IPV6, IPV6_V6ONLY, OPTVAL_T(&off), sizeof(off)) == COAP_SOCKET_ERROR)
      coap_log(LOG_WARNING,
               "coap_socket_connect_udp: setsockopt IPV6_V6ONLY: %s\n",
               coap_socket_strerror());
    break;
  default:
    coap_log(LOG_ALERT, "coap_socket_connect_udp: unsupported sa_family\n");
    break;
  }

  if (local_if && local_if->addr.sa.sa_family) {
    if (setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR, OPTVAL_T(&on), sizeof(on)) == COAP_SOCKET_ERROR)
      coap_log(LOG_WARNING,
               "coap_socket_connect_udp: setsockopt SO_REUSEADDR: %s\n",
               coap_socket_strerror());
    if (bind(sock->fd, &local_if->addr.sa, local_if->size) == COAP_SOCKET_ERROR) {
      coap_log(LOG_WARNING, "coap_socket_connect_udp: bind: %s\n",
               coap_socket_strerror());
      goto error;
    }
  }

  /* special treatment for sockets that are used for multicast communication */
  if (is_mcast) {
    if (getsockname(sock->fd, &local_addr->addr.sa, &local_addr->size) == COAP_SOCKET_ERROR) {
      coap_log(LOG_WARNING,
              "coap_socket_connect_udp: getsockname for multicast socket: %s\n",
              coap_socket_strerror());
    }
    coap_address_copy(remote_addr, &connect_addr);
    sock->flags |= COAP_SOCKET_MULTICAST;
    return 1;
  }

  if (connect(sock->fd, &connect_addr.addr.sa, connect_addr.size) == COAP_SOCKET_ERROR) {
    coap_log(LOG_WARNING, "coap_socket_connect_udp: connect: %s\n",
             coap_socket_strerror());
    goto error;
  }

  if (getsockname(sock->fd, &local_addr->addr.sa, &local_addr->size) == COAP_SOCKET_ERROR) {
    coap_log(LOG_WARNING, "coap_socket_connect_udp: getsockname: %s\n",
             coap_socket_strerror());
  }

  if (getpeername(sock->fd, &remote_addr->addr.sa, &remote_addr->size) == COAP_SOCKET_ERROR) {
    coap_log(LOG_WARNING, "coap_socket_connect_udp: getpeername: %s\n",
             coap_socket_strerror());
  }

  sock->flags |= COAP_SOCKET_CONNECTED;
  return 1;

error:
  coap_socket_close(sock);
  return 0;
}


void coap_socket_close(coap_socket_t *sock){
    if (sock->fd != COAP_INVALID_SOCKET) {
        coap_closesocket(sock->fd);
        sock->fd = COAP_INVALID_SOCKET;
    }
    sock->flags = COAP_SOCKET_EMPTY;
}


ssize_t
coap_socket_write(coap_socket_t *sock, const uint8_t *data, size_t data_len) {
  ssize_t r;

  sock->flags &= ~(COAP_SOCKET_WANT_WRITE | COAP_SOCKET_CAN_WRITE);
#ifdef _WIN32
  r = send(sock->fd, (const char *)data, (int)data_len, 0);
#else
  r = send(sock->fd, data, data_len, 0);
#endif
  if (r == COAP_SOCKET_ERROR) {
#ifdef _WIN32
    if (WSAGetLastError() == WSAEWOULDBLOCK) {
#elif EAGAIN != EWOULDBLOCK
    if (errno==EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
#else
    if (errno==EAGAIN || errno == EINTR) {
#endif
      sock->flags |= COAP_SOCKET_WANT_WRITE;
      return 0;
    }
    coap_log(LOG_WARNING, "coap_socket_write: send: %s\n",
             coap_socket_strerror());
    return -1;
  }
  if (r < (ssize_t)data_len)
    sock->flags |= COAP_SOCKET_WANT_WRITE;
  return r;
}

ssize_t
coap_socket_read(coap_socket_t *sock, uint8_t *data, size_t data_len) {
  ssize_t r;
#ifdef _WIN32
  int error;
#endif

#ifdef _WIN32
  r = recv(sock->fd, (char *)data, (int)data_len, 0);
#else
  r = recv(sock->fd, data, data_len, 0);
#endif
  if (r == 0) {
    /* graceful shutdown */
    sock->flags &= ~COAP_SOCKET_CAN_READ;
    return -1;
  } else if (r == COAP_SOCKET_ERROR) {
    sock->flags &= ~COAP_SOCKET_CAN_READ;
#ifdef _WIN32
    error = WSAGetLastError();
    if (error == WSAEWOULDBLOCK) {
#elif EAGAIN != EWOULDBLOCK
    if (errno==EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
#else
    if (errno==EAGAIN || errno == EINTR) {
#endif
      return 0;
    }
#ifdef _WIN32
    if (error != WSAECONNRESET)
#else
    if (errno != ECONNRESET)
#endif
      coap_log(LOG_WARNING, "coap_socket_read: recv: %s\n",
               coap_socket_strerror());
    return -1;
  }
  if (r < (ssize_t)data_len)
    sock->flags &= ~COAP_SOCKET_CAN_READ;
  return r;
}

#if (!defined(WITH_CONTIKI)) != ( defined(HAVE_NETINET_IN_H) || defined(HAVE_WS2TCPIP_H) )
/* define struct in6_pktinfo and struct in_pktinfo if not available
   FIXME: check with configure
*/
struct in6_pktinfo {
  struct in6_addr ipi6_addr;        /* src/dst IPv6 address */
  unsigned int ipi6_ifindex;        /* send/recv interface index */
};

#endif

#if !defined(WITH_CONTIKI) && !defined(SOL_IP)
/* Solaris expects level IPPROTO_IP for ancillary data. */
#define SOL_IP IPPROTO_IP
#endif

#ifdef __GNUC__
#define UNUSED_PARAM __attribute__ ((unused))
#else /* not a GCC */
#define UNUSED_PARAM
#endif /* GCC */

#if defined(_WIN32)
#include <mswsock.h>
static __declspec(thread) LPFN_WSARECVMSG lpWSARecvMsg = NULL;
/* Map struct WSABUF fields to their posix counterpart */
#define msghdr _WSAMSG
#define msg_name name
#define msg_namelen namelen
#define msg_iov lpBuffers
#define msg_iovlen dwBufferCount
#define msg_control Control.buf
#define msg_controllen Control.len
#define iovec _WSABUF
#define iov_base buf
#define iov_len len
#define iov_len_t u_long
#undef CMSG_DATA
#define CMSG_DATA WSA_CMSG_DATA
#define ipi_spec_dst ipi_addr
#else
#define iov_len_t size_t
#endif

ssize_t
coap_network_send(coap_socket_t *sock, const coap_session_t *session, const uint8_t *data, size_t datalen) {
  ssize_t bytes_written = 0;

  if (!coap_debug_send_packet()) {
    bytes_written = (ssize_t)datalen;
  } else if (sock->flags & COAP_SOCKET_CONNECTED) {
    bytes_written = send(sock->fd, data, datalen, 0);
  } else {

#ifndef COAP_BAD_RECVMSG
    /* a buffer large enough to hold all packet info types, ipv6 is the largest */
    char buf[CMSG_SPACE(sizeof(struct in6_pktinfo))];
    struct msghdr mhdr;
    struct iovec iov[1];
    const void *addr = &session->remote_addr.addr;

    assert(session);

    memcpy (&iov[0].iov_base, &data, sizeof (iov[0].iov_base));
    iov[0].iov_len = (iov_len_t)datalen;

    memset(buf, 0, sizeof (buf));

    memset(&mhdr, 0, sizeof(struct msghdr));
    memcpy (&mhdr.msg_name, &addr, sizeof (mhdr.msg_name));
    mhdr.msg_namelen = session->remote_addr.size;

    mhdr.msg_iov = iov;
    mhdr.msg_iovlen = 1;

    if (!coap_address_isany(&session->local_addr) && !coap_is_mcast(&session->local_addr)) switch (session->local_addr.addr.sa.sa_family) {
    case AF_INET6:
    {
      struct cmsghdr *cmsg;

      if (IN6_IS_ADDR_V4MAPPED(&session->local_addr.addr.sin6.sin6_addr)) {
#if defined(IP_PKTINFO)
        struct in_pktinfo *pktinfo;
        mhdr.msg_control = buf;
        mhdr.msg_controllen = CMSG_SPACE(sizeof(struct in_pktinfo));

        cmsg = CMSG_FIRSTHDR(&mhdr);
        cmsg->cmsg_level = SOL_IP;
        cmsg->cmsg_type = IP_PKTINFO;
        cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));

        pktinfo = (struct in_pktinfo *)CMSG_DATA(cmsg);

        pktinfo->ipi_ifindex = session->ifindex;
        memcpy(&pktinfo->ipi_spec_dst, session->local_addr.addr.sin6.sin6_addr.s6_addr + 12, sizeof(pktinfo->ipi_spec_dst));
#elif defined(IP_SENDSRCADDR)
        mhdr.msg_control = buf;
        mhdr.msg_controllen = CMSG_SPACE(sizeof(struct in_addr));

        cmsg = CMSG_FIRSTHDR(&mhdr);
        cmsg->cmsg_level = IPPROTO_IP;
        cmsg->cmsg_type = IP_SENDSRCADDR;
        cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_addr));

        memcpy(CMSG_DATA(cmsg), session->local_addr.addr.sin6.sin6_addr.s6_addr + 12, sizeof(struct in_addr));
#endif /* IP_PKTINFO */
      } else {
        struct in6_pktinfo *pktinfo;
        mhdr.msg_control = buf;
        mhdr.msg_controllen = CMSG_SPACE(sizeof(struct in6_pktinfo));

        cmsg = CMSG_FIRSTHDR(&mhdr);
        cmsg->cmsg_level = IPPROTO_IPV6;
        cmsg->cmsg_type = IPV6_PKTINFO;
        cmsg->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));

        pktinfo = (struct in6_pktinfo *)CMSG_DATA(cmsg);

        pktinfo->ipi6_ifindex = session->ifindex;
        memcpy(&pktinfo->ipi6_addr, &session->local_addr.addr.sin6.sin6_addr, sizeof(pktinfo->ipi6_addr));
      }
      break;
    }
    case AF_INET:
    {
#if defined(IP_PKTINFO)
      struct cmsghdr *cmsg;
      struct in_pktinfo *pktinfo;

      mhdr.msg_control = buf;
      mhdr.msg_controllen = CMSG_SPACE(sizeof(struct in_pktinfo));

      cmsg = CMSG_FIRSTHDR(&mhdr);
      cmsg->cmsg_level = SOL_IP;
      cmsg->cmsg_type = IP_PKTINFO;
      cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));

      pktinfo = (struct in_pktinfo *)CMSG_DATA(cmsg);

      pktinfo->ipi_ifindex = session->ifindex;
      memcpy(&pktinfo->ipi_spec_dst, &session->local_addr.addr.sin.sin_addr, sizeof(pktinfo->ipi_spec_dst));
#elif defined(IP_SENDSRCADDR)
      struct cmsghdr *cmsg;
      mhdr.msg_control = buf;
      mhdr.msg_controllen = CMSG_SPACE(sizeof(struct in_addr));

      cmsg = CMSG_FIRSTHDR(&mhdr);
      cmsg->cmsg_level = IPPROTO_IP;
      cmsg->cmsg_type = IP_SENDSRCADDR;
      cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_addr));

      memcpy(CMSG_DATA(cmsg), &session->local_addr.addr.sin.sin_addr, sizeof(struct in_addr));
#endif /* IP_PKTINFO */
      break;
    }
    default:
      /* error */
      coap_log(LOG_WARNING, "protocol not supported\n");
      bytes_written = -1;
    }
#endif /* ! COAP_BAD_RECVMSG */

#ifndef COAP_BAD_RECVMSG
    bytes_written = sendmsg(sock->fd, &mhdr, 0);
#else /* COAP_BAD_RECVMSG */
    bytes_written = sendto(sock->fd, data, datalen, 0, &session->remote_addr.addr.sa, session->remote_addr.size);
#endif /* COAP_BAD_RECVMSG */
  }

  if (bytes_written < 0)
    coap_log(LOG_CRIT, "coap_network_send: %s\n", coap_socket_strerror());

  return bytes_written;
}

#define SIN6(A) ((struct sockaddr_in6 *)(A))

void
coap_packet_get_memmapped(coap_packet_t *packet, unsigned char **address, size_t *length) {
  *address = packet->payload;
  *length = packet->length;
}

void coap_packet_set_addr(coap_packet_t *packet, const coap_address_t *src, const coap_address_t *dst) {
  coap_address_copy(&packet->src, src);
  coap_address_copy(&packet->dst, dst);
}

ssize_t
coap_network_read(coap_socket_t *sock, coap_packet_t *packet) {
  ssize_t len = -1;

  assert(sock);
  assert(packet);

  if ((sock->flags & COAP_SOCKET_CAN_READ) == 0) {
    return -1;
  } else {
    /* clear has-data flag */
    sock->flags &= ~COAP_SOCKET_CAN_READ;
  }

  if (sock->flags & COAP_SOCKET_CONNECTED) {

    len = recv(sock->fd, packet->payload, COAP_RXBUFFER_SIZE, 0);

    if (len < 0) {
      if (errno == ECONNREFUSED) {
        /* client-side ICMP destination unreachable, ignore it */
        coap_log(LOG_WARNING, "coap_network_read: unreachable\n");
        return -2;
      }
      coap_log(LOG_WARNING, "coap_network_read: %s\n", coap_socket_strerror());
      goto error;
    } else if (len > 0) {
      packet->length = (size_t)len;
    }
  } else {

#if !defined(WITH_CONTIKI)
#ifndef COAP_BAD_RECVMSG
    /* a buffer large enough to hold all packet info types, ipv6 is the largest */
    char buf[CMSG_SPACE(sizeof(struct in6_pktinfo))];
    struct msghdr mhdr;
    struct iovec iov[1];

    iov[0].iov_base = packet->payload;
    iov[0].iov_len = (iov_len_t)COAP_RXBUFFER_SIZE;

    memset(&mhdr, 0, sizeof(struct msghdr));

    mhdr.msg_name = (struct sockaddr*)&packet->src.addr;
    mhdr.msg_namelen = sizeof(packet->src.addr);

    mhdr.msg_iov = iov;
    mhdr.msg_iovlen = 1;

    mhdr.msg_control = buf;
    mhdr.msg_controllen = sizeof(buf);

#if defined(_WIN32)
    if (!lpWSARecvMsg) {
      GUID wsaid = WSAID_WSARECVMSG;
      DWORD cbBytesReturned = 0;
      if (WSAIoctl(sock->fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &wsaid, sizeof(wsaid), &lpWSARecvMsg, sizeof(lpWSARecvMsg), &cbBytesReturned, NULL, NULL) != 0) {
        coap_log(LOG_WARNING, "coap_network_read: no WSARecvMsg\n");
        return -1;
      }
    }
    r = lpWSARecvMsg(sock->fd, &mhdr, &dwNumberOfBytesRecvd, NULL /* LPWSAOVERLAPPED */, NULL /* LPWSAOVERLAPPED_COMPLETION_ROUTINE */);
    if (r == 0)
      len = (ssize_t)dwNumberOfBytesRecvd;
#else
    len = recvmsg(sock->fd, &mhdr, 0);
#endif

#else /* COAP_BAD_RECVMSG */
    packet->src.size = packet->src.size;
    len = recvfrom(sock->fd, packet->payload, COAP_RXBUFFER_SIZE, 0, &packet->src.addr.sa, &packet->src.size);
#endif /* COAP_BAD_RECVMSG */

    if (len < 0) {
#ifdef _WIN32
      if (WSAGetLastError() == WSAECONNRESET) {
#else
      if (errno == ECONNREFUSED) {
#endif
        /* server-side ICMP destination unreachable, ignore it. The destination address is in msg_name. */
        return 0;
      }
      coap_log(LOG_WARNING, "coap_network_read: %s\n", coap_socket_strerror());
      goto error;
    } else {
#ifndef COAP_BAD_RECVMSG
      struct cmsghdr *cmsg;

      packet->src.size = mhdr.msg_namelen;
      packet->length = (size_t)len;

      /* Walk through ancillary data records until the local interface
       * is found where the data was received. */
      for (cmsg = CMSG_FIRSTHDR(&mhdr); cmsg; cmsg = CMSG_NXTHDR(&mhdr, cmsg)) {

        /* get the local interface for IPv6 */
        if (cmsg->cmsg_level == IPPROTO_IPV6 && cmsg->cmsg_type == IPV6_PKTINFO) {
          union {
            uint8_t *c;
            struct in6_pktinfo *p;
          } u;
          u.c = CMSG_DATA(cmsg);
          packet->ifindex = (int)(u.p->ipi6_ifindex);
          memcpy(&packet->dst.addr.sin6.sin6_addr, &u.p->ipi6_addr, sizeof(struct in6_addr));
          break;
        }

        /* local interface for IPv4 */
#if defined(IP_PKTINFO)
        if (cmsg->cmsg_level == SOL_IP && cmsg->cmsg_type == IP_PKTINFO) {
          union {
            uint8_t *c;
            struct in_pktinfo *p;
          } u;
          u.c = CMSG_DATA(cmsg);
          packet->ifindex = u.p->ipi_ifindex;
          if (packet->dst.addr.sa.sa_family == AF_INET6) {
            memset(packet->dst.addr.sin6.sin6_addr.s6_addr, 0, 10);
            packet->dst.addr.sin6.sin6_addr.s6_addr[10] = 0xff;
            packet->dst.addr.sin6.sin6_addr.s6_addr[11] = 0xff;
            memcpy(packet->dst.addr.sin6.sin6_addr.s6_addr + 12, &u.p->ipi_addr, sizeof(struct in_addr));
          } else {
            memcpy(&packet->dst.addr.sin.sin_addr, &u.p->ipi_addr, sizeof(struct in_addr));
          }
          break;
        }
#elif defined(IP_RECVDSTADDR)
        if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_RECVDSTADDR) {
          packet->ifindex = 0;
          memcpy(&packet->dst.addr.sin.sin_addr, CMSG_DATA(cmsg), sizeof(struct in_addr));
          break;
        }
#endif /* IP_PKTINFO */
      }
#else /* COAP_BAD_RECVMSG */
      packet->length = (size_t)len;
      packet->ifindex = 0;
      if (getsockname(sock->fd, &packet->dst.addr.sa, &packet->dst.size) < 0) {
         coap_log(LOG_DEBUG, "Cannot determine local port\n");
         goto error;
      }
#endif /* COAP_BAD_RECVMSG */
    }
#endif /* !defined(WITH_CONTIKI) */

  }


  if (len >= 0)
    return len;

error:
  return -1;
}

unsigned int
coap_write(coap_context_t *ctx,
           coap_socket_t *sockets[],
           unsigned int max_sockets,
           unsigned int *num_sockets,
           coap_tick_t now)
{
  coap_queue_t *nextpdu;
  coap_endpoint_t *ep;
  coap_session_t *s;
  coap_tick_t session_timeout;
  coap_tick_t timeout = 0;
  coap_session_t *tmp;

  *num_sockets = 0;

  /* Check to see if we need to send off any Observe requests */
  coap_check_notify(ctx);

  if (ctx->session_timeout > 0)
    session_timeout = ctx->session_timeout * COAP_TICKS_PER_SECOND;
  else
    session_timeout = COAP_DEFAULT_SESSION_TIMEOUT * COAP_TICKS_PER_SECOND;

  LL_FOREACH(ctx->endpoint, ep) {
    if (ep->sock.flags & (COAP_SOCKET_WANT_READ | COAP_SOCKET_WANT_WRITE | COAP_SOCKET_WANT_ACCEPT)) {
      if (*num_sockets < max_sockets)
        sockets[(*num_sockets)++] = &ep->sock;
    }
    LL_FOREACH_SAFE(ep->sessions, s, tmp) {
      if (s->type == COAP_SESSION_TYPE_SERVER && s->ref == 0 &&
          s->delayqueue == NULL &&
          (s->last_rx_tx + session_timeout <= now ||
           s->state == COAP_SESSION_STATE_NONE)) {
        coap_session_free(s);
      } else {
        if (s->type == COAP_SESSION_TYPE_SERVER && s->ref == 0 && s->delayqueue == NULL) {
          coap_tick_t s_timeout = (s->last_rx_tx + session_timeout) - now;
          if (timeout == 0 || s_timeout < timeout)
            timeout = s_timeout;
        }
        if (s->sock.flags & (COAP_SOCKET_WANT_READ | COAP_SOCKET_WANT_WRITE)) {
          if (*num_sockets < max_sockets)
            sockets[(*num_sockets)++] = &s->sock;
        }
      }
    }
  }
  LL_FOREACH_SAFE(ctx->sessions, s, tmp) {
    if (
        s->type == COAP_SESSION_TYPE_CLIENT
     && COAP_PROTO_RELIABLE(s->proto)
     && s->state == COAP_SESSION_STATE_ESTABLISHED
     && ctx->ping_timeout > 0
    ) {
      coap_tick_t s_timeout;
      if (s->last_rx_tx + ctx->ping_timeout * COAP_TICKS_PER_SECOND <= now) {
        if ((s->last_ping > 0 && s->last_pong < s->last_ping)
          || coap_session_send_ping(s) == COAP_INVALID_TID)
        {
          /* Make sure the session object is not deleted in the callback */
          coap_session_reference(s);
          coap_session_disconnected(s, COAP_NACK_NOT_DELIVERABLE);
          coap_session_release(s);
          continue;
        }
        s->last_rx_tx = now;
        s->last_ping = now;
      }
      s_timeout = (s->last_rx_tx + ctx->ping_timeout * COAP_TICKS_PER_SECOND) - now;
      if (timeout == 0 || s_timeout < timeout)
        timeout = s_timeout;
    }

    if (
        s->type == COAP_SESSION_TYPE_CLIENT
     && COAP_PROTO_RELIABLE(s->proto)
     && s->state == COAP_SESSION_STATE_CSM
     && ctx->csm_timeout > 0
    ) {
      coap_tick_t s_timeout;
      if (s->csm_tx == 0) {
        s->csm_tx = now;
      } else if (s->csm_tx + ctx->csm_timeout * COAP_TICKS_PER_SECOND <= now) {
        /* Make sure the session object is not deleted in the callback */
        coap_session_reference(s);
        coap_session_disconnected(s, COAP_NACK_NOT_DELIVERABLE);
        coap_session_release(s);
        continue;
      }
      s_timeout = (s->csm_tx + ctx->csm_timeout * COAP_TICKS_PER_SECOND) - now;
      if (timeout == 0 || s_timeout < timeout)
        timeout = s_timeout;
    }

    if (s->sock.flags & (COAP_SOCKET_WANT_READ | COAP_SOCKET_WANT_WRITE | COAP_SOCKET_WANT_CONNECT)) {
      if (*num_sockets < max_sockets)
        sockets[(*num_sockets)++] = &s->sock;
    }
  }

  nextpdu = coap_peek_next(ctx);

  while (nextpdu && now >= ctx->sendqueue_basetime && nextpdu->t <= now - ctx->sendqueue_basetime) {
    coap_retransmit(ctx, coap_pop_next(ctx));
    nextpdu = coap_peek_next(ctx);
  }

  if (nextpdu && (timeout == 0 || nextpdu->t - ( now - ctx->sendqueue_basetime ) < timeout))
    timeout = nextpdu->t - (now - ctx->sendqueue_basetime);

  if (ctx->dtls_context) {
    if (coap_dtls_is_context_timeout()) {
      coap_tick_t tls_timeout = coap_dtls_get_context_timeout(ctx->dtls_context);
      if (tls_timeout > 0) {
        if (tls_timeout < now + COAP_TICKS_PER_SECOND / 10)
          tls_timeout = now + COAP_TICKS_PER_SECOND / 10;
        coap_log(LOG_DEBUG, "** DTLS global timeout set to %dms\n",
                 (int)((tls_timeout - now) * 1000 / COAP_TICKS_PER_SECOND));
        if (timeout == 0 || tls_timeout - now < timeout)
          timeout = tls_timeout - now;
      }
    } else {
      LL_FOREACH(ctx->endpoint, ep) {
        if (ep->proto == COAP_PROTO_DTLS) {
          LL_FOREACH(ep->sessions, s) {
            if (s->proto == COAP_PROTO_DTLS && s->tls) {
              coap_tick_t tls_timeout = coap_dtls_get_timeout(s);
              while (tls_timeout > 0 && tls_timeout <= now) {
                coap_log(LOG_DEBUG, "** %s: DTLS retransmit timeout\n",
                         coap_session_str(s));
                coap_dtls_handle_timeout(s);
                if (s->tls)
                  tls_timeout = coap_dtls_get_timeout(s);
                else {
                  tls_timeout = 0;
                  timeout = 1;
                }
              }
              if (tls_timeout > 0 && (timeout == 0 || tls_timeout - now < timeout))
                timeout = tls_timeout - now;
            }
          }
        }
      }
      LL_FOREACH(ctx->sessions, s) {
        if (s->proto == COAP_PROTO_DTLS && s->tls) {
          coap_tick_t tls_timeout = coap_dtls_get_timeout(s);
          while (tls_timeout > 0 && tls_timeout <= now) {
            coap_log(LOG_DEBUG, "** %s: DTLS retransmit timeout\n", coap_session_str(s));
            coap_dtls_handle_timeout(s);
            if (s->tls)
              tls_timeout = coap_dtls_get_timeout(s);
            else {
              tls_timeout = 0;
              timeout = 1;
            }
          }
          if (tls_timeout > 0 && (timeout == 0 || tls_timeout - now < timeout))
            timeout = tls_timeout - now;
        }
      }
    }
  }

  return (unsigned int)((timeout * 1000 + COAP_TICKS_PER_SECOND - 1) / COAP_TICKS_PER_SECOND);
}

int
coap_run_once(coap_context_t *ctx, unsigned timeout_ms) {
  fd_set readfds, writefds, exceptfds;
  coap_fd_t nfds = 0;
  struct timeval tv;
  coap_tick_t before, now;
  int result;
  coap_socket_t *sockets[64];
  unsigned int num_sockets = 0, i, timeout;

  coap_ticks(&before);

  timeout = coap_write(ctx, sockets, (unsigned int)(sizeof(sockets) / sizeof(sockets[0])), &num_sockets, before);
  if (timeout == 0 || timeout_ms < timeout)
    timeout = timeout_ms;

  FD_ZERO(&readfds);
  FD_ZERO(&writefds);
  FD_ZERO(&exceptfds);
  for (i = 0; i < num_sockets; i++) {
    if (sockets[i]->fd + 1 > nfds)
      nfds = sockets[i]->fd + 1;
    if (sockets[i]->flags & COAP_SOCKET_WANT_READ)
      FD_SET(sockets[i]->fd, &readfds);
    if (sockets[i]->flags & COAP_SOCKET_WANT_WRITE)
      FD_SET(sockets[i]->fd, &writefds);
    if (sockets[i]->flags & COAP_SOCKET_WANT_ACCEPT)
      FD_SET(sockets[i]->fd, &readfds);
    if (sockets[i]->flags & COAP_SOCKET_WANT_CONNECT) {
      FD_SET(sockets[i]->fd, &writefds);
      FD_SET(sockets[i]->fd, &exceptfds);
    }
  }

  if ( timeout > 0 ) {
    tv.tv_usec = (timeout % 1000) * 1000;
    tv.tv_sec = (long)(timeout / 1000);
  }

  result = select(nfds, &readfds, &writefds, &exceptfds, timeout > 0 ? &tv : NULL);

  if (result < 0) {   /* error */
    if (errno != EINTR) {

      coap_log(LOG_DEBUG, "%s", coap_socket_strerror());
      return -1;
    }
  }

  if (result > 0) {
    for (i = 0; i < num_sockets; i++) {
      if ((sockets[i]->flags & COAP_SOCKET_WANT_READ) && FD_ISSET(sockets[i]->fd, &readfds))
        sockets[i]->flags |= COAP_SOCKET_CAN_READ;
      if ((sockets[i]->flags & COAP_SOCKET_WANT_ACCEPT) && FD_ISSET(sockets[i]->fd, &readfds))
        sockets[i]->flags |= COAP_SOCKET_CAN_ACCEPT;
      if ((sockets[i]->flags & COAP_SOCKET_WANT_WRITE) && FD_ISSET(sockets[i]->fd, &writefds))
        sockets[i]->flags |= COAP_SOCKET_CAN_WRITE;
      if ((sockets[i]->flags & COAP_SOCKET_WANT_CONNECT) && (FD_ISSET(sockets[i]->fd, &writefds) || FD_ISSET(sockets[i]->fd, &exceptfds)))
        sockets[i]->flags |= COAP_SOCKET_CAN_CONNECT;
    }
  }

  coap_ticks(&now);
  coap_read(ctx, now);

  return (int)(((now - before) * 1000) / COAP_TICKS_PER_SECOND);
}

static const char *coap_socket_format_errno(int error) {
  return strerror(error);
}


const char *coap_socket_strerror(void) {
  return strerror(errno);
}


ssize_t coap_socket_send(
    coap_socket_t *sock,
    coap_session_t *session,
    const uint8_t *data, 
    size_t data_len
){
  return session->context->network_send(sock, session, data, data_len);
}

#undef SIN6
