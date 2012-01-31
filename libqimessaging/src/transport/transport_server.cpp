/*
** transport-server.cpp
** Login : <hcuche@hcuche-de>
** Started on  Wed Jan 11 10:19:42 2012 Herve Cuche
** $Id$
**
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Herve Cuche
*/

#include <iostream>
#include <string>
#include <cstring>
#include <qi/log.hpp>

#include <event2/util.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <arpa/inet.h>

#include <qimessaging/transport/transport_server.hpp>

namespace qi {
#define MAX_LINE 16384

// Fake unique ID for client
static int uniqueId = 0;


static void readcb(struct bufferevent *bev,
                   void *context)
{
  ClientConnection *cc = static_cast<ClientConnection*>(context);
  cc->_parent->readcb(bev, context);
}

static void writecb(struct bufferevent* bev, void* context)
{
  ClientConnection *cc = static_cast<ClientConnection*>(context);
  cc->_parent->writecb(bev, context);
}

static void eventcb(struct bufferevent *bev,
                    short events,
                    void *context)
{
  ClientConnection *cc = static_cast<ClientConnection*>(context);
  cc->_parent->eventcb(bev, events, context);
}


class TransportServerPrivate
{
public:
  TransportServerPrivate() {}

  ~TransportServerPrivate()
  {
    event_base_free(base);
  }

  TransportServerDelegate *tsd;
  struct event_base       *base;
};

void accept(evutil_socket_t listener,
            short events,
            void *context)
{
  TransportServer *ts = static_cast<TransportServer*>(context);
  ts->accept(listener, events, context);
}


TransportServer::TransportServer()
{
  _p = new TransportServerPrivate();
}

TransportServer::~TransportServer()
{
  delete _p;
}

void TransportServer::accept(evutil_socket_t listener,
                             short events,
                             void *context)
{
  struct event_base *base = _p->base;
  struct sockaddr_storage ss;
  socklen_t slen = sizeof(ss);

  int fd = ::accept(listener, (struct sockaddr*)&ss, &slen);

  if (fd < 0)
  {
    qiLogError("qimessaging.TransportServer") << "Gateway: Could not accept client" << std::endl;
  }
  else if (fd > FD_SETSIZE)
  {
    ::close(fd);
  }
  else
  {
    struct bufferevent *bev;
    evutil_make_socket_nonblocking(fd);
    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

    std::stringstream ss;
    ss << uniqueId++;
    ClientConnection *cc = new ClientConnection(ss.str(), bev, this);
    clientConnected[cc->_id] = cc;
    bufferevent_setcb(bev, ::qi::readcb, ::qi::writecb, ::qi::eventcb, cc);

    bufferevent_setwatermark(bev, EV_READ, 0, MAX_LINE);
    bufferevent_enable(bev, EV_READ|EV_WRITE);
  }
}


void TransportServer::readcb(struct bufferevent *bev,
                             void *context)
{
  char buf[1024];
  memset(buf, '\0', 1024);
  size_t n;
  struct evbuffer *input = bufferevent_get_input(bev);

  ClientConnection *cc = static_cast<ClientConnection*>(context);

  while ((n = evbuffer_remove(input, buf, sizeof(buf))) > 0)
  {
    std::stringstream newMsg;
    newMsg << cc->_id << "." << buf;
    _p->tsd->onRead(newMsg.str());
  }
}

void TransportServer::writecb(struct bufferevent* bev, void* context)
{
  (void) bev;

  ClientConnection *cc = static_cast<ClientConnection*>(context);
  _p->tsd->onWrite();
}

void TransportServer::eventcb(struct bufferevent *bev,
                              short events,
                              void *context)
{
  if (events & BEV_EVENT_EOF)
  {
    // connection has been closed, do any clean up here
    ClientConnection *cc = static_cast<ClientConnection*>(context);
    qiLogInfo("qimessaging.TransportServer") << "Client ID " << cc->_id << " has closed connection." << std::endl;
    clientConnected.erase(clientConnected.find(cc->_id));
  }
  else if (events & BEV_EVENT_ERROR)
  {
    // check errno to see what error occurred
    qiLogInfo("qimessaging.TransportServer")  << "check errno to see what error occurred" << std::endl;
  }
  else if (events & BEV_EVENT_TIMEOUT)
  {
    // must be a timeout event handle, handle it
    qiLogInfo("qimessaging.TransportServer")  << "must be a timeout event handle, handle it" << std::endl;
  }
}

void TransportServer::start(const std::string &address,
                            unsigned short port,
                            struct event_base *base)
{
  evutil_socket_t sock;
  struct event*   sockEvent;

  // get tcp socket
  if ((sock = ::socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    qiLogError("qimessaging.transportserver") << "Could not get socket" << std::endl;
    return;
  }
  evutil_make_socket_nonblocking(sock);

  // get valid IP
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  if ((addr.sin_addr.s_addr = inet_addr(address.c_str())) == INADDR_NONE)
  {
    qiLogError("qimessaging.transportserver") << "Provided IP is not valid" << std::endl;
    return;
  }

  // bind socket
  if (::bind(sock, (struct sockaddr*)&addr, sizeof (addr)) == -1)
  {
    qiLogError("qimessaging.transportserver") << "Could not bind socket" << std::endl;
    return;
  }

  //listen on the socket
  if (::listen(sock, SOMAXCONN) == -1)
  {
    qiLogError("qimessaging.transportserver") << "Could not listen on socket" << std::endl;
    return;
  }

  _p->base = base;
  sockEvent = event_new(_p->base, sock, EV_READ | EV_PERSIST,
                        ::qi::accept, this);

  event_add(sockEvent, NULL);
}

void TransportServer::setDelegate(TransportServerDelegate *delegate)
{
  _p->tsd = delegate;
}


}
