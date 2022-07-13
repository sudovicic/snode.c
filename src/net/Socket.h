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

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include "core/Descriptor.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include "core/system/socket.h" // IWYU pragma: export

#include <functional> // IWYU pragma: export

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

namespace net {

    template <typename SocketAddressT>
    class Socket : public core::Descriptor {
    public:
        using SocketAddress = SocketAddressT;

        Socket();
        Socket(int fd); // cppcheck-suppress noExplicitConstructor
        Socket(int domain, int type, int protocol);

        Socket& operator=(int fd);

        virtual ~Socket() = default;

        enum SOCK { NONE = 0, NONBLOCK = SOCK_NONBLOCK, CLOEXIT = SOCK_CLOEXEC };

    protected:
        int create(SOCK flags);

        virtual void setSockopt();

    public:
        int open(SOCK flags = SOCK::NONE);
        int bind(const SocketAddress& bindAddress);

        bool isValid() const;

        int reuseAddress();

        int getSockname(SocketAddress& socketAddress);
        int getPeername(SocketAddress& socketAddress);

        int getSockError();

        int setSockopt(int level, int optname, const void* optval, socklen_t optlen);
        int getSockopt(int level, int optname, void* optval, socklen_t* optlen);

        ssize_t sendFd(const SocketAddress& destAddress, void* ptr, size_t nbytes, int sendfd);
        ssize_t recvFd(void* ptr, size_t nbytes, int* recvfd);

        enum SHUT { WR = SHUT_WR, RD = SHUT_RD, RDWR = SHUT_RDWR };

        void shutdown(SHUT how);

        const SocketAddress& getBindAddress() const;

    protected:
        SocketAddress bindAddress{};

    private:
        int domain{};
        int type{};
        int protocol{};
    };

} // namespace net

#endif // NET_SOCKET_H
