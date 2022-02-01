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

#ifndef NET_L2_STREAM_TLS_CLIENTCONFIG_H
#define NET_L2_STREAM_TLS_CLIENTCONFIG_H

#include "net/ConfigTls.h"              // IWYU pragma: export
#include "net/l2/stream/ClientConfig.h" // IWYU pragma: export

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <string>

#endif // DOXYGEN_SHOULD_SKIP_THIS

namespace net::l2::stream::tls {

    class ClientConfig
        : public net::l2::stream::ClientConfig
        , public net::ConfigTls {
    public:
        explicit ClientConfig(const std::string& name)
            : net::l2::stream::ClientConfig(name)
            , net::ConfigTls(baseSc) {
            parse();
        }
    };

} // namespace net::l2::stream::tls

#endif // NET_L2_STREAM_TLS_CLIENTCONFIG_H
