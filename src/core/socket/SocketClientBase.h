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

#ifndef CORE_SOCKET_CLIENTSOCKET_H
#define CORE_SOCKET_CLIENTSOCKET_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <functional>

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

namespace core::socket {

    template <typename SocketT>
    class SocketClientBase {
    public:
        using Socket = SocketT;

        virtual void connect(const typename Socket::SocketAddress& remoteAddress,
                             const typename Socket::SocketAddress& bindAddress,
                             const std::function<void(int)>& onError) const = 0;

        virtual void connect(const typename Socket::SocketAddress& remoteAddress, const std::function<void(int)>& onError) const = 0;
    };

} // namespace core::socket

#endif // CORE_SOCKET_CLIENTSOCKET_H