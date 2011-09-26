/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#include <qimessaging/qi.h>
#include "src/messaging/client_impl.hpp"

qi_client_t *qi_client_create(const char *name) {
  qi::detail::ClientImpl *pclient = new qi::detail::ClientImpl(name);
  return static_cast<qi_client_t *>(pclient);
}

qi_client_t *qi_client_create_with_context(const char *name, qi_context_t *ctx) {
  qi::Context *pctx = static_cast<qi::Context *>(ctx);
  qi::detail::ClientImpl *pclient = new qi::detail::ClientImpl(name, pctx);
  return static_cast<qi_client_t *>(pclient);
}

int qi_client_connect(qi_client_t *client, const char *address)
{
  qi::detail::ClientImpl  *pclient  = static_cast<qi::detail::ClientImpl *>(client);
  pclient->connect(address);
  //todo: try catch
  return 0;
}

void qi_client_destroy(qi_client_t *client)
{
  qi::detail::ClientImpl  *pclient  = static_cast<qi::detail::ClientImpl *>(client);
  delete pclient;
}

int qi_client_call(qi_client_t *client, const char *method, qi_message_t *msg, qi_message_t *ret)
{
  qi::detail::ClientImpl  *pclient  = static_cast<qi::detail::ClientImpl *>(client);
  qi::Message *preturn  = (qi::Message *)ret;
  qi::Message *pmessage = (qi::Message *)msg;

  pclient->call(std::string(method), *pmessage, *preturn);
  //todo: try catch
  return 0;
}




