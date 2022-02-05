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

#include "ListenEventReceiver.h"

#include "core/EventDispatcher.h"
#include "core/EventLoop.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

namespace core {

    ListenEventReceiver::ListenEventReceiver()
        : DescriptorEventReceiver(EventLoop::instance().getEventDispatcher().getDescriptorEventDispatcher(core::EventDispatcher::DISP_TYPE::WR), 0) {
    }

    void ListenEventReceiver::dispatchEvent() {
        listenEvent();
    }

    void ListenEventReceiver::timeoutEvent() {
        // do nothing
    }

    bool ListenEventReceiver::continueImmediately() const {
        return false;
    }

} // namespace core