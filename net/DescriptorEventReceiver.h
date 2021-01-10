/*
 * snode.c - a slim toolkit for network communication
 * Copyright (C) 2020 Volker Christian <me@vchrist.at>
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

#ifndef DESCRIPTOREVENTRECEIVER_H
#define DESCRIPTOREVENTRECEIVER_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <climits>
#include <ctime>

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

namespace net {

    class ObservationCounter {
    public:
        bool isObserved() {
            return observationCounter > 0;
        }

    protected:
        int observationCounter = 0;
        struct timeval lastTriggered {};
    };

    class DescriptorEventReceiver : virtual public ObservationCounter {
    protected:
        class TIMEOUT {
        public:
            static const long IGNORE = -2;
            static const long DEFAULT = -1;
            static const long DISABLE = LONG_MAX;
        };

        explicit DescriptorEventReceiver(long timeout)
            : maxInactivity(timeout) {
        }

    public:
        DescriptorEventReceiver(const DescriptorEventReceiver&) = delete;
        DescriptorEventReceiver& operator=(const DescriptorEventReceiver&) = delete;

    protected:
        virtual ~DescriptorEventReceiver() = default;

        void setTimeout(long timeout, long defaultTimeout) { // -3: do not change, -2: set default, -1 disable, ...
            if (timeout != TIMEOUT::IGNORE) {
                if (timeout != TIMEOUT::DEFAULT) {
                    this->maxInactivity = timeout;
                } else {
                    this->maxInactivity = defaultTimeout;
                }
            }
        }

        bool isEnabled() const {
            return _enabled;
        }

        bool isSuspended() const {
            return _suspended;
        }

        int fd = -1;

    private:
        virtual void dispatchEvent() = 0;

        virtual void timeout() {
        }

        void enabled(int fd) {
            this->fd = fd;
            ObservationCounter::observationCounter++;
            _enabled = true;
            lastTriggered = {time(nullptr), 0};
        }

        void disabled() {
            this->fd = -1;
            ObservationCounter::observationCounter--;
            _enabled = false;
        }

        void suspended() {
            _suspended = true;
        }

        void resumed() {
            _suspended = false;
            lastTriggered = {time(nullptr), 0};
        }

        struct timeval getTimeout() const {
            return {maxInactivity, 0};
        }

        struct timeval getLastTriggered() {
            return lastTriggered;
        }

        void triggered(struct timeval _lastTriggered = {time(nullptr), 0}) {
            lastTriggered = _lastTriggered;
        }

        virtual void enable(int fd, long timeout) = 0;
        virtual void disable() = 0;

        virtual void suspend() = 0;
        virtual void resume() = 0;

        virtual void unobserved() = 0;

        bool _enabled = false;
        bool _suspended = false;

        long maxInactivity = LONG_MAX;

        friend class DescriptorEventDispatcher;
    };

} // namespace net

#endif // DESCRIPTOREVENTRECEIVER_H