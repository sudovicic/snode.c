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

#ifndef NET_CONFIGREMOTE_H
#define NET_CONFIGREMOTE_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

namespace CLI {
    class App;
    class Option;
} // namespace CLI

#endif // DOXYGEN_SHOULD_SKIP_THIS

namespace net::config {

    template <typename SocketAddressT>
    class ConfigRemote {
        using SocketAddress = SocketAddressT;

    protected:
        ConfigRemote(CLI::App* baseSc);
        virtual ~ConfigRemote() = default;

    public:
        const SocketAddress& getRemoteAddress();
        void setRemoteAddress(const SocketAddress& remoteAddress);

    protected:
        void require(CLI::Option* opt);
        void require(CLI::Option* opt1, CLI::Option* opt2);

        CLI::App* addressSc = nullptr;
        SocketAddress address;

    private:
        virtual void updateFromCommandLine() = 0;

        bool initialized = false;
        bool updated = false;
    };

} // namespace net::config

#endif // CONFIGREMOTE_H