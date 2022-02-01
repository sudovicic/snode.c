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

#ifndef NET_IN6_STREAM_CONFIGCONNECT_H
#define NET_IN6_STREAM_CONFIGCONNECT_H

#include "net/in6/SocketAddress.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS

namespace CLI {
    class App;
    class Option;
} // namespace CLI

#include <cstdint>
#include <string>

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

namespace net::in6::stream {

    class ConfigConnect {
    public:
        explicit ConfigConnect(CLI::App* baseSc);

        const std::string& getConnectHost() const;

        uint16_t getConnectPort() const;

        SocketAddress getConnectAddress() const;

    protected:
        void required() const;

        CLI::App* connectSc = nullptr;
        CLI::Option* connectHostOpt = nullptr;
        CLI::Option* connectPortOpt = nullptr;

    private:
        std::string connectHost = "";
        uint16_t connectPort = 0;
    };

} // namespace net::in6::stream

#endif // NET_IN6_STREAM_CONFIGCONNECT_H
