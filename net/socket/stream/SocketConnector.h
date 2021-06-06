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

#ifndef NET_SOCKET_STREAM_SOCKETCONNECTOR_H
#define NET_SOCKET_STREAM_SOCKETCONNECTOR_H

#include "net/ConnectEventReceiver.h"
#include "net/socket/Socket.h"
#include "net/socket/stream/SocketContextFactory.h"
#include "net/system/socket.h"

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

namespace net::socket::stream {

    template <typename SocketConnectionT>
    class SocketConnector
        : public ConnectEventReceiver
        , public SocketConnectionT::Socket {
        SocketConnector() = delete;
        SocketConnector(const SocketConnector&) = delete;
        SocketConnector& operator=(const SocketConnector&) = delete;

    public:
        using SocketConnection = SocketConnectionT;
        using Socket = typename SocketConnection::Socket;
        using SocketAddress = typename Socket::SocketAddress;

        SocketConnector(const std::shared_ptr<const SocketContextFactory>& socketProtocolFactory,
                        const std::function<void(const SocketAddress& localAddress, const SocketAddress& remoteAddress)>& onConnect,
                        const std::function<void(SocketConnection* socketConnection)>& onConnected,
                        const std::function<void(SocketConnection* socketConnection)>& onDisconnect,
                        const std::map<std::string, std::any>& options)
            : socketProtocolFactory(socketProtocolFactory)
            , options(options)
            , onConnect(onConnect)
            , onConnected(onConnected)
            , onDisconnect(onDisconnect) {
        }

        virtual ~SocketConnector() = default;

        void connect(const SocketAddress& remoteAddress, const SocketAddress& bindAddress, const std::function<void(int err)>& onError) {
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
                                int ret =
                                    net::system::connect(Socket::getFd(), &remoteAddress.getSockAddr(), remoteAddress.getSockAddrLen());

                                if (ret == 0 || errno == EINPROGRESS) {
                                    ConnectEventReceiver::enable(Socket::getFd());
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

            int err = net::system::getsockopt(Socket::getFd(), SOL_SOCKET, SO_ERROR, &cErrno, &cErrnoLen);

            if (err == 0) {
                if (cErrno != EINPROGRESS) {
                    if (cErrno == 0) {
                        typename SocketAddress::SockAddr localAddress{};
                        socklen_t localAddressLength = sizeof(localAddress);

                        typename SocketAddress::SockAddr remoteAddress{};
                        socklen_t remoteAddressLength = sizeof(remoteAddress);

                        if (net::system::getsockname(Socket::getFd(), reinterpret_cast<sockaddr*>(&localAddress), &localAddressLength) ==
                                0 &&
                            net::system::getpeername(Socket::getFd(), reinterpret_cast<sockaddr*>(&remoteAddress), &remoteAddressLength) ==
                                0) {
                            socketConnection = new SocketConnection(Socket::getFd(),
                                                                    socketProtocolFactory,
                                                                    SocketAddress(localAddress),
                                                                    SocketAddress(remoteAddress),
                                                                    onConnect,
                                                                    onDisconnect);
                            SocketConnector::dontClose(true);
                            SocketConnector::ConnectEventReceiver::disable();

                            onConnected(socketConnection);
                            onError(0);
                        } else {
                            SocketConnector::ConnectEventReceiver::disable();
                            cErrno = errno;
                            onError(errno);
                        }
                    } else {
                        SocketConnector::ConnectEventReceiver::disable();
                        errno = cErrno;
                        onError(errno);
                    }
                }
            } else {
                SocketConnector::ConnectEventReceiver::disable();
                cErrno = errno;
                onError(errno);
            }
        }

        void unobserved() override {
            destruct();
        }

    protected:
        void destruct() {
            delete this;
        }

    private:
        std::shared_ptr<const SocketContextFactory> socketProtocolFactory = nullptr;

    protected:
        std::function<void(int err)> onError;

        std::map<std::string, std::any> options;

    private:
        std::function<void(const SocketAddress& localAddress, const SocketAddress& remoteAddress)> onConnect;
        std::function<void(SocketConnection* socketConnection)> onDestruct;
        std::function<void(SocketConnection* socketConnection)> onConnected;
        std::function<void(SocketConnection* socketConnection)> onDisconnect;

        SocketConnection* socketConnection = nullptr;
    };

} // namespace net::socket::stream

#endif // NET_SOCKET_STREAM_SOCKETCONNECTOR_H
