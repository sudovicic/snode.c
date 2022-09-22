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

#ifndef MQTT_PACKETS_DISCONNECT_H
#define MQTT_PACKETS_DISCONNECT_H

#include "mqtt/ControlPacket.h"

namespace mqtt {
    class ControlPacketFactory;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#endif // DOXYGEN_SHOUÖD_SKIP_THIS

namespace mqtt::packets {

    class Disconnect : public mqtt::ControlPacket {
    public:
        explicit Disconnect(mqtt::ControlPacketFactory& controlPacketFactory);
        Disconnect(const Disconnect&) = default;

        Disconnect& operator=(const Disconnect&) = default;

        ~Disconnect();
    };

} // namespace mqtt::packets

#endif // MQTT_PACKETS_DISCONNECT_H
