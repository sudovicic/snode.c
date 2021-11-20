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

#ifndef NET_L2CAP_STREAM_SOCKET_H
#define NET_L2CAP_STREAM_SOCKET_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#include "core/socket/Socket.h"
#include "net/l2cap/L2CapAddress.h" // IWYU pragma: export

namespace net::l2::stream {

    class Socket : public core::socket::Socket<net::l2::L2CapAddress> {
    protected:
        int create(int flags = 0) override;

    public:
        using SocketAddress = net::l2::L2CapAddress;
    };

} // namespace net::l2::stream

#endif // NET_L2CAP_STREAM_SOCKET_H
