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

#ifndef CORE_SOCKET_STREAM_SOCKETCONNECTOR_H
#define CORE_SOCKET_STREAM_SOCKETCONNECTOR_H

#include "core/ConnectEventReceiver.h"
#include "core/system/socket.h"

namespace core::socket::stream {
    class SocketContextFactory;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <any>
#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

namespace core::socket::stream {

    template <typename SocketConnectionT>
    class SocketConnector
        : public SocketConnectionT::Socket
        , public ConnectEventReceiver {
        SocketConnector() = delete;
        SocketConnector(const SocketConnector&) = delete;
        SocketConnector& operator=(const SocketConnector&) = delete;

    public:
        using SocketConnection = SocketConnectionT;
        using Socket = typename SocketConnection::Socket;
        using SocketAddress = typename Socket::SocketAddress;

        SocketConnector(const std::shared_ptr<SocketContextFactory>& socketContextFactory,
                        const std::function<void(const SocketAddress&, const SocketAddress&)>& onConnect,
                        const std::function<void(SocketConnection*)>& onConnected,
                        const std::function<void(SocketConnection*)>& onDisconnect,
                        const std::map<std::string, std::any>& options)
            : socketContextFactory(socketContextFactory)
            , options(options)
            , onConnect(onConnect)
            , onConnected(onConnected)
            , onDisconnect(onDisconnect) {
        }

        virtual ~SocketConnector() = default;

        void connect(const SocketAddress& remoteAddress, const SocketAddress& bindAddress, const std::function<void(int)>& onError) {
            this->onError = onError;

            Socket::open(
                [this, &bindAddress, &remoteAddress, &onError](int errnum) -> void {
                    if (errnum > 0) {
                        onError(errnum);
                        destruct();
                    } else {
                        Socket::bind(bindAddress, [this, &remoteAddress, &onError](int errnum) -> void {
                            if (errnum > 0) {
                                onError(errnum);
                                destruct();
                            } else {
                                int ret = core::system::connect(
                                    SocketConnector::getFd(), &remoteAddress.getSockAddr(), remoteAddress.getSockAddrLen());

                                if (ret == 0 || errno == EINPROGRESS) {
                                    ConnectEventReceiver::enable(SocketConnector::getFd());
                                    SocketConnector::dontClose(true);
                                } else {
                                    onError(errno);
                                    destruct();
                                }
                            }
                        });
                    }
                },
                SOCK_NONBLOCK);
        }

    private:
        void connectEvent() override {
            int cErrno = -1;
            socklen_t cErrnoLen = sizeof(cErrno);

            int err = core::system::getsockopt(SocketConnector::getFd(), SOL_SOCKET, SO_ERROR, &cErrno, &cErrnoLen);

            if (err == 0) {
                errno = cErrno;
                if (errno != EINPROGRESS) {
                    if (errno == 0) {
                        typename SocketAddress::SockAddr localAddress{};
                        socklen_t localAddressLength = sizeof(localAddress);

                        typename SocketAddress::SockAddr remoteAddress{};
                        socklen_t remoteAddressLength = sizeof(remoteAddress);

                        if (core::system::getsockname(
                                SocketConnector::getFd(), reinterpret_cast<sockaddr*>(&localAddress), &localAddressLength) == 0 &&
                            core::system::getpeername(
                                SocketConnector::getFd(), reinterpret_cast<sockaddr*>(&remoteAddress), &remoteAddressLength) == 0) {
                            SocketConnection* socketConnection = new SocketConnection(SocketConnector::getFd(),
                                                                                      socketContextFactory,
                                                                                      SocketAddress(localAddress),
                                                                                      SocketAddress(remoteAddress),
                                                                                      onConnect,
                                                                                      onDisconnect);
                            SocketConnector::ConnectEventReceiver::disable();

                            onConnected(socketConnection);
                            onError(0);
                        } else {
                            SocketConnector::ConnectEventReceiver::disable();
                            onError(errno);
                        }
                    } else {
                        SocketConnector::ConnectEventReceiver::disable();
                        onError(errno);
                    }
                } else {
                    // connect() still in progress
                }
            } else {
                SocketConnector::ConnectEventReceiver::disable();
                onError(errno);
            }
        }

        void unobservedEvent() override {
            destruct();
        }

    protected:
        void destruct() {
            delete this;
        }

    private:
        std::shared_ptr<SocketContextFactory> socketContextFactory = nullptr;

    protected:
        std::function<void(int err)> onError;
        std::map<std::string, std::any> options;

    private:
        std::function<void(const SocketAddress&, const SocketAddress&)> onConnect;
        std::function<void(SocketConnection*)> onDestruct;
        std::function<void(SocketConnection*)> onConnected;
        std::function<void(SocketConnection*)> onDisconnect;
    };

} // namespace core::socket::stream

#endif // CORE_SOCKET_STREAM_SOCKETCONNECTOR_H
