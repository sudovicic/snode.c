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

#ifndef DESCRIPTOREVENTDISPATCHER_H
#define DESCRIPTOREVENTDISPATCHER_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <list>
#include <map>
#include <sys/time.h> // for timeval

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

namespace net {

    class DescriptorEventReceiver;
    class FdSet;

    class DescriptorEventDispatcher {
    public:
    private:
        class DescriptorEventReceiverList : public std::list<DescriptorEventReceiver*> {
        public:
            using std::list<DescriptorEventReceiver*>::begin;
            using std::list<DescriptorEventReceiver*>::end;
            using std::list<DescriptorEventReceiver*>::front;

            bool contains(DescriptorEventReceiver* descriptorEventReceiver);
        };

    public:
        explicit DescriptorEventDispatcher(FdSet& fdSet, long maxInactivity); // NOLINT(google-runtime-references)
        DescriptorEventDispatcher(const DescriptorEventDispatcher&) = delete;

        DescriptorEventDispatcher& operator=(const DescriptorEventDispatcher&) = delete;

        void enable(DescriptorEventReceiver* eventReceiver, int fd);
        void disable(DescriptorEventReceiver* eventReceiver, int fd);
        void suspend(DescriptorEventReceiver* eventReceiver, int fd);
        void resume(DescriptorEventReceiver* eventReceiver, int fd);

        unsigned long getEventCounter() const;
        long getTimeout() const;

    private:
        int getMaxFd() const;

        struct timeval dispatchActiveEvents(struct timeval currentTime);

        struct timeval observeEnabledEvents();
        void unobserveDisabledEvents();
        void releaseUnobservedEvents();
        void disableObservedEvents();

        std::map<int, DescriptorEventReceiverList> enabledEventReceiver;
        std::map<int, DescriptorEventReceiverList> observedEventReceiver;
        std::map<int, DescriptorEventReceiverList> disabledEventReceiver;
        DescriptorEventReceiverList unobservedEventReceiver;

        FdSet& fdSet;

        long maxInactivity;

        unsigned long eventCounter = 0;

        friend class EventLoop;
    }; // namespace net

} // namespace net

#endif // DESCRIPTOREVENTDISPATCHER_H