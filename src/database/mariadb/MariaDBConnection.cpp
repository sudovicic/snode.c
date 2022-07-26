/*
 * snode.c - a slim toolkit for network communication
 * Copyright (C) 2020, 2021, 2022 Volker Christian <me@vchrist.at>
 *               2021, 2022 Daniel Flockert
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

#include "database/mariadb/MariaDBConnection.h"

#include "database/mariadb/MariaDBClient.h"
#include "database/mariadb/MariaDBCommand.h" // for MariaDBCommand
#include "database/mariadb/commands/async/MariaDBConnectCommand.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include "log/Logger.h"

#include <algorithm>
#include <memory>
#include <string>
#include <utility> // for move

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

namespace database::mariadb {

    MariaDBConnection::MariaDBConnection(MariaDBClient* mariaDBClient, const MariaDBConnectionDetails& connectionDetails)
        : ReadEventReceiver("MariaDBConnectionRead", core::DescriptorEventReceiver::TIMEOUT::DISABLE)
        , WriteEventReceiver("MariaDBConnectionWrite", core::DescriptorEventReceiver::TIMEOUT::DISABLE)
        , ExceptionalConditionEventReceiver("MariaDBConnectionExceptional", core::DescriptorEventReceiver::TIMEOUT::DISABLE)
        , mariaDBClient(mariaDBClient)
        , mysql(mysql_init(nullptr))
        , commandStartEvent("MariaDBCommandStartEvent", this) {
        mysql_options(mysql, MYSQL_OPT_NONBLOCK, nullptr);

        execute_async(std::move(MariaDBCommandSequence().execute_async(new database::mariadb::commands::async::MariaDBConnectCommand(
            connectionDetails,
            [this](void) -> void {
                if (mysql_errno(mysql) == 0) {
                    int fd = mysql_get_socket(mysql);

                    VLOG(0) << "Got valid descriptor: " << fd;

                    ReadEventReceiver::enable(fd);
                    WriteEventReceiver::enable(fd);
                    ExceptionalConditionEventReceiver::enable(fd);

                    ReadEventReceiver::suspend();
                    WriteEventReceiver::suspend();
                    ExceptionalConditionEventReceiver::suspend();

                    connected = true;
                } else {
                    VLOG(0) << "Got no valid descriptor: " << mysql_error(mysql) << ", " << mysql_errno(mysql);
                }
            },
            [](void) -> void {
                VLOG(0) << "Connect success";
            },
            [](const std::string& errorString, unsigned int errorNumber) -> void {
                VLOG(0) << "Connect error: " << errorString << " : " << errorNumber;
            }))));
    }

    MariaDBConnection::~MariaDBConnection() {
        for (MariaDBCommandSequence& mariaDBCommandSequence : commandSequenceQueue) {
            for (MariaDBCommand* mariaDBCommand : mariaDBCommandSequence.sequence()) {
                mariaDBCommand->commandError(mysql_error(mysql), mysql_errno(mysql));

                delete mariaDBCommand;
            }
        }

        if (mariaDBClient != nullptr) {
            mariaDBClient->connectionVanished();
        }

        mysql_close(mysql);
        mysql_library_end();
    }

    MariaDBCommandSequence& MariaDBConnection::execute_async(MariaDBCommandSequence&& commandSequence) {
        if (currentCommand == nullptr && commandSequenceQueue.empty()) {
            commandStartEvent.publish();
        }

        commandSequenceQueue.emplace_back(std::move(commandSequence));

        return commandSequenceQueue.back();
    }

    void MariaDBConnection::execute_sync(MariaDBCommand* mariaDBCommand) {
        mariaDBCommand->commandStart(mysql, utils::Timeval::currentTime());

        if (mysql_errno(mysql) == 0) {
            if (mariaDBCommand->commandCompleted()) {
            }
        } else {
            mariaDBCommand->commandError(mysql_error(mysql), mysql_errno(mysql));
        }

        delete mariaDBCommand;
    }

    void MariaDBConnection::commandStart(const utils::Timeval& currentTime) {
        if (!commandSequenceQueue.empty()) {
            currentCommand = commandSequenceQueue.front().nextCommand();
            currentCommand->setMariaDBConnection(this);

            // VLOG(0) << "Start: " << currentCommand->commandInfo();

            int status = currentCommand->commandStart(mysql, currentTime);
            checkStatus(status);
        } else if (mariaDBClient != nullptr) {
            if (ReadEventReceiver::isSuspended()) {
                ReadEventReceiver::resume();
            }
        } else {
            ReadEventReceiver::disable();
            WriteEventReceiver::disable();
            ExceptionalConditionEventReceiver::disable();
        }
    }

    void MariaDBConnection::commandContinue(int status) {
        if (currentCommand != nullptr) {
            // VLOG(0) << "Continue: " << currentCommand->commandInfo();
            int currentStatus = currentCommand->commandContinue(status);
            checkStatus(currentStatus);
        } else if ((status & MYSQL_WAIT_READ) != 0 && commandSequenceQueue.empty()) {
            VLOG(0) << "Read-Event but no command in queue: Disabling EventReceivers";

            ReadEventReceiver::disable();
            WriteEventReceiver::disable();
            ExceptionalConditionEventReceiver::disable();
        }
    }

    void MariaDBConnection::commandCompleted() {
        VLOG(0) << "Completed: " << currentCommand->commandInfo();
        commandSequenceQueue.front().commandCompleted();

        if (commandSequenceQueue.front().empty()) {
            commandSequenceQueue.pop_front();
        }

        delete currentCommand;
        currentCommand = nullptr;
    }

    void MariaDBConnection::unmanaged() {
        mariaDBClient = nullptr;
    }

    void MariaDBConnection::checkStatus(int status) {
        if (connected) {
            if ((status & MYSQL_WAIT_READ) != 0) {
                if (ReadEventReceiver::isSuspended()) {
                    ReadEventReceiver::resume();
                }
            } else if (!ReadEventReceiver::isSuspended()) {
                ReadEventReceiver::suspend();
            }

            if ((status & MYSQL_WAIT_WRITE) != 0) {
                if (WriteEventReceiver::isSuspended()) {
                    WriteEventReceiver::resume();
                }
            } else if (!WriteEventReceiver::isSuspended()) {
                WriteEventReceiver::suspend();
            }

            if ((status & MYSQL_WAIT_EXCEPT) != 0) {
                if (ExceptionalConditionEventReceiver::isSuspended()) {
                    ExceptionalConditionEventReceiver::resume();
                }
            } else if (!ExceptionalConditionEventReceiver::isSuspended()) {
                ExceptionalConditionEventReceiver::suspend();
            }

            if ((status & MYSQL_WAIT_TIMEOUT) != 0) {
                ReadEventReceiver::setTimeout(mysql_get_timeout_value(mysql));
                //                WriteEventReceiver::setTimeout(mysql_get_timeout_value(mysql));
                //                ExceptionalConditionEventReceiver::setTimeout(mysql_get_timeout_value(mysql));
            } else {
                ReadEventReceiver::setTimeout(core::DescriptorEventReceiver::TIMEOUT::DEFAULT);
                //                WriteEventReceiver::setTimeout(core::DescriptorEventReceiver::TIMEOUT::DEFAULT);
                //                ExceptionalConditionEventReceiver::setTimeout(core::DescriptorEventReceiver::TIMEOUT::DEFAULT);
            }

            if (status == 0) {
                if (mysql_errno(mysql) == 0) {
                    if (currentCommand->commandCompleted()) {
                        commandCompleted();
                    }
                } else {
                    currentCommand->commandError(mysql_error(mysql), mysql_errno(mysql));
                    commandCompleted();
                }
                commandStartEvent.publish();
            }
        } else {
            currentCommand->commandError(mysql_error(mysql), mysql_errno(mysql));
            commandCompleted();
            delete this;
        }
    }

    void MariaDBConnection::readEvent() {
        commandContinue(MYSQL_WAIT_READ);
    }

    void MariaDBConnection::writeEvent() {
        commandContinue(MYSQL_WAIT_WRITE);
    }

    void MariaDBConnection::outOfBandEvent() {
        commandContinue(MYSQL_WAIT_EXCEPT);
    }

    void MariaDBConnection::readTimeout() {
        commandContinue(MYSQL_WAIT_TIMEOUT);
    }

    void MariaDBConnection::writeTimeout() {
        commandContinue(MYSQL_WAIT_TIMEOUT);
    }

    void MariaDBConnection::outOfBandTimeout() {
        commandContinue(MYSQL_WAIT_TIMEOUT);
    }

    void MariaDBConnection::unobservedEvent() {
        delete this;
    }

    MariaDBCommandStartEvent::MariaDBCommandStartEvent(const std::string& name, MariaDBConnection* mariaDBConnection)
        : core::EventReceiver(name)
        , mariaDBConnection(mariaDBConnection) {
    }

    MariaDBCommandStartEvent::~MariaDBCommandStartEvent() {
        core::EventReceiver::unPublish();
    }

    void MariaDBCommandStartEvent::publish() {
        core::EventReceiver::publish();
    }

    void MariaDBCommandStartEvent::event(const utils::Timeval& currentTime) {
        mariaDBConnection->commandStart(currentTime);
    }

} // namespace database::mariadb
