/*
 * snode.c - a slim toolkit for network communication
 * Copyright (C) 2020, 2021 Volker Christian <me@vchrist.at>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "web/websocket/server/SocketContext.h"

#include "web/websocket/server/SubProtocol.h"
#include "web/websocket/server/SubProtocolFactorySelector.h"

namespace net::socket::stream {
    class SocketConnection;
} // namespace net::socket::stream

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <memory> // for allocator

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#define CLOSE_SOCKET_TIMEOUT 10

namespace web::websocket::server {

    SocketContext::SocketContext(net::socket::stream::SocketConnection* socketConnection, web::websocket::server::SubProtocol* subProtocol)
        : web::websocket::SocketContext<web::websocket::server::SubProtocol>(socketConnection, subProtocol, Role::SERVER) {
        subProtocol->setSocketContext(this);
    }

    SocketContext::~SocketContext() {
        SubProtocolFactorySelector::instance()->select(subProtocol->getName())->destroy(subProtocol);
    }

} // namespace web::websocket::server
