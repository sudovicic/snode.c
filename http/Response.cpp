#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <filesystem>
#include <numeric>

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#include "HTTPServerContext.h"
#include "HTTPStatusCodes.h"
#include "MimeTypes.h"
#include "Response.h"
#include "WebApp.h"
#include "file/FileReader.h"
#include "httputils.h"

Response::Response(HTTPServerContext* httpContext)
    : httpContext(httpContext) {
}

void Response::enqueue(const char* buf, size_t len) {
    if (!headersSent && !headersSentInProgress) {
        headersSentInProgress = true;
        sendHeader();
        headersSentInProgress = false;
        headersSent = true;
    }

    httpContext->sendResponseData(buf, len);

    if (headersSent) {
        contentSent += len;
        if (contentSent == contentLength) {
            httpContext->requestCompleted();
        }
    }
}

void Response::enqueue(const std::string& str) {
    this->enqueue(str.c_str(), str.size());
}

Response& Response::status(int status) {
    this->responseStatus = status;

    return *this;
}

Response& Response::append(const std::string& field, const std::string& value) {
    std::map<std::string, std::string>::iterator it = this->headers.find(field);

    if (it != this->headers.end()) {
        it->second += ", " + value;
    } else {
        this->set(field, value);
    }

    return *this;
}

Response& Response::set(const std::map<std::string, std::string>& map, bool overwrite) {
    for (const std::pair<const std::string, std::string>& header : map) {
        this->set(header.first, header.second, overwrite);
    }

    return *this;
}

Response& Response::set(const std::string& field, const std::string& value, bool overwrite) {
    if (overwrite) {
        this->headers.insert_or_assign(field, value);
    } else {
        this->headers.insert({field, value});
    }

    if (field == "Content-Length") {
        contentLength = std::stol(value);
    }

    return *this;
}

Response& Response::type(const std::string& type) {
    this->set({{"Content-Type", type}});

    return *this;
}

Response& Response::cookie(const std::string& name, const std::string& value, const std::map<std::string, std::string>& options) {
    this->cookies.insert({name, ResponseCookie(value, options)});

    return *this;
}

Response& Response::clearCookie(const std::string& name, const std::map<std::string, std::string>& options) {
    std::map<std::string, std::string> opts = options;

    opts.erase("Max-Age");
    time_t time = 0;
    opts["Expires"] = httputils::to_http_date(gmtime(&time));

    this->cookie(name, "", opts);

    return *this;
}

void Response::send(const char* buffer, size_t size) {
    if (size > 0) {
        headers.insert({"Content-Type", "application/octet-stream"});
    }
    headers.insert_or_assign("Content-Length", std::to_string(size));

    this->enqueue(buffer, size);
}

void Response::send(const std::string& text) {
    if (text.size() > 0) {
        headers.insert({"Content-Type", "text/html; charset=utf-8"});
    }
    this->send(text.c_str(), text.size());
}

void Response::sendHeader() {
    this->enqueue("HTTP/1.1 " + std::to_string(responseStatus) + " " + HTTPStatusCode::reason(responseStatus) + "\r\n");
    this->enqueue("Date: " + httputils::to_http_date() + "\r\n");

    headers.insert({{"Cache-Control", "public, max-age=0"}, {"Accept-Ranges", "bytes"}, {"X-Powered-By", "snode.c"}});

    for (const std::pair<const std::string, std::string>& header : headers) {
        this->enqueue(header.first + ": " + header.second + "\r\n");
    }

    for (const std::pair<const std::string, Response::ResponseCookie>& cookie : cookies) {
        std::string cookieString =
            std::accumulate(cookie.second.options.begin(), cookie.second.options.end(), cookie.first + "=" + cookie.second.value,
                            [](const std::string& str, const std::pair<const std::string&, const std::string&> option) -> std::string {
                                return str + "; " + option.first + (!option.second.empty() ? "=" + option.second : "");
                            });
        this->enqueue("Set-Cookie: " + cookieString + "\r\n");
    }

    this->enqueue("\r\n");

    contentLength = std::stoi(headers.find("Content-Length")->second);
}

void Response::stop() {
    if (fileReader != nullptr) {
        fileReader->stop();
        fileReader = nullptr;
    }
}

void Response::sendFile(const std::string& file, const std::function<void(int err)>& onError) {
    std::string absolutFileName = httpContext->webApp.getRootDir() + file;

    if (std::filesystem::exists(absolutFileName)) {
        std::error_code ec;
        absolutFileName = std::filesystem::canonical(absolutFileName);

        if (absolutFileName.rfind(httpContext->webApp.getRootDir(), 0) == 0 && std::filesystem::is_regular_file(absolutFileName, ec) &&
            !ec) {
            headers.insert({{"Content-Type", MimeTypes::contentType(absolutFileName)},
                                   {"Last-Modified", httputils::file_mod_http_date(absolutFileName)}});
            headers.insert_or_assign("Content-Length", std::to_string(std::filesystem::file_size(absolutFileName)));

            fileReader = FileReader::read(
                absolutFileName,
                [this](char* data, int length) -> void {
                    this->enqueue(data, length);
                },
                [this, onError](int err) -> void {
                    if (onError) {
                        onError(err);
                    }
                    if (err != 0) {
                        httpContext->terminateConnection();
                    }
                });
        } else {
            responseStatus = 403;
            errno = EACCES;
            this->end();
            this->httpContext->terminateConnection();
            if (onError) {
                onError(EACCES);
            }
        }
    } else {
        responseStatus = 404;
        errno = ENOENT;
        this->end();
        this->httpContext->terminateConnection();
        if (onError) {
            onError(ENOENT);
        }
    }
}

void Response::download(const std::string& file, const std::function<void(int err)>& onError) {
    std::string name = file;

    if (name[0] == '/') {
        name.erase(0, 1);
    }

    this->download(file, name, onError);
}

void Response::download(const std::string& file, const std::string& name, const std::function<void(int err)>& onError) {
    this->set({{"Content-Disposition", "attachment; filename=\"" + name + "\""}}).sendFile(file, onError);
}

void Response::redirect(const std::string& name) {
    this->redirect(302, name);
}

void Response::redirect(int status, const std::string& name) {
    this->status(status).set({{"Location", name}});
    this->end();
}

void Response::sendStatus(int status) {
    this->status(status).send(HTTPStatusCode::reason(status));
}

void Response::end() {
    this->send("");
    this->httpContext->requestCompleted();
}

void Response::reset() {
    headersSent = false;
    headersSentInProgress = false;
    contentSent = 0;
    responseStatus = 200;
    contentLength = 0;
    headers.clear();
    cookies.clear();
    stop();
}
