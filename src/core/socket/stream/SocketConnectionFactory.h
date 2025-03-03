/*
 * snode.c - a slim toolkit for network communication
 * Copyright (C) 2020, 2021, 2022 Volker Christian <me@vchrist.at>
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

#ifndef CORE_SOCKET_STREAM_SOCKETCONNECTIONESTABLISHER_H
#define CORE_SOCKET_STREAM_SOCKETCONNECTIONESTABLISHER_H

namespace core::socket {
    class SocketContextFactory;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include "log/Logger.h"

#include <functional>
#include <memory>

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

namespace core::socket::stream {

    template <typename BaseSocketT, typename SocketConnectionT>
    class SocketConnectionFactory {
    private:
        using BaseSocket = BaseSocketT;
        using Socket = typename BaseSocket::Socket;

    protected:
        using SocketConnection = SocketConnectionT;

    public:
        SocketConnectionFactory(const std::shared_ptr<core::socket::SocketContextFactory>& socketContextFactory,
                                const std::function<void(SocketConnection*)>& onConnect,
                                const std::function<void(SocketConnection*)>& onConnected,
                                const std::function<void(SocketConnection*)>& onDisconnect)
            : socketContextFactory(socketContextFactory)
            , onConnect(onConnect)
            , onConnected(onConnected)
            , onDisconnect(onDisconnect) {
        }

        using Config = typename BaseSocket::Config;
        using SocketAddress = typename Socket::SocketAddress;

        void create(Socket& socket, const std::shared_ptr<Config>& config) {
            if (socket.isValid()) {
                socket.dontClose();
                SocketAddress localAddress{};
                SocketAddress remoteAddress{};
                if (socket.getSockname(localAddress) == 0 && socket.getPeername(remoteAddress) == 0) {
                    onConnected(new SocketConnection(socket.getFd(),
                                                     socketContextFactory,
                                                     SocketAddress(localAddress),
                                                     SocketAddress(remoteAddress),
                                                     onConnect,
                                                     onDisconnect,
                                                     config->getReadTimeout(),
                                                     config->getWriteTimeout(),
                                                     config->getReadBlockSize(),
                                                     config->getWriteBlockSize(),
                                                     config->getTerminateTimeout()));
                } else {
                    PLOG(ERROR) << "getsockname";
                }
            }
        }

    protected:
        std::shared_ptr<core::socket::SocketContextFactory> socketContextFactory = nullptr;

        std::function<void(SocketConnection*)> onConnect;
        std::function<void(SocketConnection*)> onConnected;
        std::function<void(SocketConnection*)> onDisconnect;
    };

} // namespace core::socket::stream

#endif // CORE_SOCKET_STREAM_SOCKETCONNECTIONESTABLISHER_H
