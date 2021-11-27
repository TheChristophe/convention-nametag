#ifndef CONVENTION_NAMETAG_SERVER_HPP
#define CONVENTION_NAMETAG_SERVER_HPP

#include <App.h>
#include <libusockets.h>
#include <nlohmann/json.hpp>

#include <string>

class WebServer {
    public:
    explicit WebServer() = default;
    ~WebServer() = default;

    void run();
    void halt();

    private:
    us_listen_socket_t *_socket{};

    short _port{ 8080 };
};

#endif