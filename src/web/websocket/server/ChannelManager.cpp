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

#include "web/websocket/server/ChannelManager.h"

#include "web/websocket/server/SubProtocol.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

namespace web::websocket::server {

    ChannelManager* ChannelManager::channelManager = nullptr;

    ChannelManager::~ChannelManager() {
        ChannelManager::channelManager = nullptr;
    }

    ChannelManager* ChannelManager::instance() {
        return ChannelManager::channelManager == nullptr ? ChannelManager::channelManager = new ChannelManager()
                                                         : ChannelManager::channelManager;
    }

    void ChannelManager::subscribe(SubProtocol* subProtocol, std::string channel) {
        if (channel.empty()) {
            channel = subProtocol->getName();
        }

        if (subProtocol->channel != channel) {
            std::string newChannel = subProtocol->getName() + "/" + channel;

            channels[newChannel].insert(subProtocol);

            if (!subProtocol->channel.empty()) {
                unsubscribe(subProtocol);
            }

            subProtocol->channel = newChannel;
        }
    }

    void ChannelManager::unsubscribe(SubProtocol* subProtocol) {
        if (channels.contains(subProtocol->channel)) {
            channels[subProtocol->channel].erase(subProtocol);

            if (channels[subProtocol->channel].size() == 0) {
                channels.erase(subProtocol->channel);
                if (channels.size() == 0) {
                    delete this;
                }
            }
        }
    }

    void ChannelManager::sendBroadcast(const std::string& channel,
                                       const char* message,
                                       std::size_t messageLength,
                                       const SubProtocol* excludedClient) {
        if (channels.contains(channel)) {
            for (SubProtocol* client : channels[channel]) {
                if (client != excludedClient) {
                    client->sendMessage(message, messageLength);
                }
            }
        }
    }

    void ChannelManager::sendBroadcast(const std::string& channel, const std::string& message, const SubProtocol* excludedClient) {
        if (channels.contains(channel)) {
            for (SubProtocol* client : channels[channel]) {
                if (client != excludedClient) {
                    client->sendMessage(message);
                }
            }
        }
    }

    void ChannelManager::sendBroadcastStart(const std::string& channel,
                                            const char* message,
                                            std::size_t messageLength,
                                            const SubProtocol* excludedClient) {
        if (channels.contains(channel)) {
            for (SubProtocol* client : channels[channel]) {
                if (client != excludedClient) {
                    client->sendMessageStart(message, messageLength);
                }
            }
        }
    }

    void ChannelManager::sendBroadcastStart(const std::string& channel, const std::string& message, const SubProtocol* excludedClient) {
        sendBroadcastStart(channel, message.data(), message.length(), excludedClient);
    }

    void ChannelManager::sendBroadcastFrame(const std::string& channel,
                                            const char* message,
                                            std::size_t messageLength,
                                            const SubProtocol* excludedClient) {
        if (channels.contains(channel)) {
            for (SubProtocol* client : channels[channel]) {
                if (client != excludedClient) {
                    client->sendMessageFrame(message, messageLength);
                }
            }
        }
    }

    void ChannelManager::sendBroadcastFrame(const std::string& channel, const std::string& message, const SubProtocol* excludedClient) {
        sendBroadcastFrame(channel, message.data(), message.length(), excludedClient);
    }

    void ChannelManager::sendBroadcastEnd(const std::string& channel,
                                          const char* message,
                                          std::size_t messageLength,
                                          const SubProtocol* excludedClient) {
        if (channels.contains(channel)) {
            for (SubProtocol* client : channels[channel]) {
                if (client != excludedClient) {
                    client->sendMessageEnd(message, messageLength);
                }
            }
        }
    }

    void ChannelManager::sendBroadcastEnd(const std::string& channel, const std::string& message, const SubProtocol* excludedClient) {
        sendBroadcastEnd(channel, message.data(), message.length(), excludedClient);
    }

    void ChannelManager::forEachClient(const std::string& channel,
                                       const std::function<void(SubProtocol*)>& sendToClient,
                                       const SubProtocol* excludedClient) {
        if (channels.contains(channel)) {
            for (SubProtocol* client : channels[channel]) {
                if (client != excludedClient) {
                    sendToClient(client);
                }
            }
        }
    }

} // namespace web::websocket::server