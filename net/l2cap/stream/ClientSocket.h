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

#ifndef NET_L2CAP_STREAM_CLIENTSOCKET_H
#define NET_L2CAP_STREAM_CLIENTSOCKET_H

#include "net/l2cap/stream/Socket.h" // IWYU pragma: export

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <functional>

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

namespace net::l2::stream {

    class ClientSocket {
    public:
        using Socket = net::l2::stream::Socket;
        using SocketAddress = Socket::SocketAddress;

        virtual void
        connect(const SocketAddress& remoteAddress, const SocketAddress& bindAddress, const std::function<void(int)>& onError) const = 0;
        virtual void connect(const SocketAddress& remoteAddress, const std::function<void(int)>& onError) const = 0;

        void connect(const std::string& address, uint16_t psm, const std::function<void(int)>& onError) {
            connect(SocketAddress(address, psm), onError);
        }

        void connect(const std::string& address, uint16_t psm, const std::string& bindAddress, const std::function<void(int)>& onError) {
            connect(SocketAddress(address, psm), SocketAddress(bindAddress), onError);
        }

        void connect(const std::string& address,
                     uint16_t psm,
                     const std::string& bindAddress,
                     uint16_t bindPsm,
                     const std::function<void(int)>& onError) {
            connect(SocketAddress(address, psm), SocketAddress(bindAddress, bindPsm), onError);
        }
    };

} // namespace net::l2::stream

#endif // NET_L2CAP_STREAM_CLIENTSOCKET_H
