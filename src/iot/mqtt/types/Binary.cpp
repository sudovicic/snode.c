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

#include "iot/mqtt/types/Binary.h"

#include "iot/mqtt/SocketContext.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#endif // DOXYGEN_SHOUÖD_SKIP_THIS

namespace iot::mqtt::types {

    Binary::Binary(iot::mqtt::SocketContext* socketContext)
        : iot::mqtt::types::TypeBase(socketContext)
        , length(socketContext) {
    }

    Binary::~Binary() {
    }

    std::size_t Binary::construct() {
        std::size_t consumed = 0;

        switch (state) {
            case 0:
                consumed = length.construct();
                if (length.isCompleted()) {
                    state++;
                    needed = stillNeeded = length.getValue();
                    data.resize(static_cast<std::vector<char>::size_type>(stillNeeded), '\0');
                    completed = stillNeeded == 0;
                } else {
                    error = length.isError();
                }
                break;
            case 1:
                consumed = socketContext->readFromPeer(data.data() + needed - stillNeeded, static_cast<std::size_t>(stillNeeded));
                stillNeeded -= consumed;
                completed = stillNeeded == 0;
                break;
        }

        return consumed;
    }

    std::vector<char>& Binary::getValue() {
        return data;
    }

    void Binary::reset() {
        state = 0;
        needed = 0;
        stillNeeded = 0;
        length.reset();
        data.clear();

        iot::mqtt::types::TypeBase::reset();
    }

} // namespace iot::mqtt::types
