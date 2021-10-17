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

#ifndef WEB_HTTP_SOCKETCONTEXTFACTORY_H
#define WEB_HTTP_SOCKETCONTEXTFACTORY_H

#include "net/socket/stream/SocketContextFactory.h"

namespace net::socket::stream {
    class SocketConnection;
} // namespace net::socket::stream

namespace web::http {
    template <template <typename RequestT, typename ResponseT> class SocketContextT, typename RequestT, typename ResponseT>
    class SocketContextFactory : public net::socket::stream::SocketContextFactory {
    public:
        using SocketContext = SocketContextT<RequestT, ResponseT>;
        using Request = RequestT;
        using Response = ResponseT;

        SocketContextFactory() = default;

        ~SocketContextFactory() override = default;

        SocketContextFactory(const SocketContextFactory&) = delete;
        SocketContextFactory& operator=(const SocketContextFactory&) = delete;

        net::socket::stream::SocketContext* create(net::socket::stream::SocketConnection* socketConnection) override = 0;
    };
} // namespace web::http

#endif // WEB_HTTP_ SOCKETCONTEXTFACTORY_H