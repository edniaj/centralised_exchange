// SocketManager.h
#pragma once

#include <string>
#include <cstdint>


class SocketManager {
private:
    int server_fd;
    int epoll_fd;

    bool set_non_blocking(int fd);
    bool bind_and_listen();

public:
    SocketManager();
    ~SocketManager();

    bool setup(int port);
    bool add_to_epoll(int socket_fd, uint32_t events);
    bool remove_from_epoll(int socket_fd);
    int accept_new_connection();

    int get_server_fd() { return server_fd; }
    int get_epoll_fd() { return epoll_fd; }
};
