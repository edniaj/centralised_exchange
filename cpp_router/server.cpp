#include "server.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

namespace myserver
{

    // Write the implementation for Server interface in the headerfile.
    Server::Server(int port) : server_fd_(-1), epoll_fd_(-1) // set value to attribute similar to python construtor
    {
        // Initialize server_address
        memset(&server_address_, 0, sizeof(server_address_));
        server_address_.sin_family = AF_INET;
        server_address_.sin_addr.s_addr = inet_addr("127.0.0.1");
        server_address_.sin_port = htons(port); // host byte order to network byte order, not sure why this isnt implicit. also its BYTE. from arpa
    }

    Server::~Server()
    {
        /**
         * @brief
         * TODO - We have to gracefully close the epoll, server, and ALL clients ports.
         *
         */
        if (server_fd_ != -1)
            close(server_fd_);
        if (epoll_fd_ != -1)
            close(epoll_fd_);
    }

    void Server::start()
    {
        if (!setup_server_and_epoll())
        {
            std::cerr << "Failed to set up server and epoll" << std::endl;
            return;
        }
        run_event_loop();
    }

    bool Server::setup_server_and_epoll()
    {
        std::cout << "\n   Creating file descriptor" << std::endl;
        server_fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (server_fd_ == -1)
        {
            std::cerr << "Terminating ... failed to create socket" << std::endl;
            return false;
        }
        print_success("Created FD for SOCKET SERVER");

        if (!set_non_blocking(server_fd_))
            return false;

        epoll_fd_ = epoll_create1(0);
        if (epoll_fd_ == -1)
        {
            std::cerr << "Terminating ... failed to create epoll FD" << std::endl;
            return false;
        }
        print_success("Created FD for EPOLL");

        if (!add_socket_to_epoll(server_fd_, EPOLLIN))
        {
            std::cerr << "Failed to add server FD into EPOLLFD" << std::endl;
            return false;
        }
        print_success("Added server_fd into epoll_fd");

        return bind_and_listen();
    }

    bool Server::bind_and_listen()
    {
        std::cout << "   Binding socket to server IP and port" << std::endl;
        if (bind(server_fd_, (struct sockaddr *)&server_address_, sizeof(server_address_)) < 0)
        {
            std::cerr << "Terminating ... failed to bind socket: " << strerror(errno) << std::endl;
            return false;
        }
        print_success("Bound socket to port");

        std::cout << "   Set Listening to Server socket" << std::endl;
        if (listen(server_fd_, PENDING_CONNECTION_BACKLOG) < 0)
        {
            std::cerr << "Terminating ... socket can't listen" << std::endl;
            return false;
        }
        print_success("Listening on server fd");
        return true;
    }

    bool Server::set_non_blocking(int fd)
    {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1)
            return false;
        flags |= O_NONBLOCK;
        if (fcntl(fd, F_SETFL, flags) == -1)
            return false;
        print_success("Set non blocking to fd");
        return true;
    }

    bool Server::add_socket_to_epoll(int socket_fd, uint32_t events)
    {
        struct epoll_event event;
        event.events = events;
        event.data.fd = socket_fd;
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket_fd, &event) == -1)
        {
            std::cerr << "Failed to add socket to epoll" << std::endl;
            return false;
        }
        return true;
    }

    void Server::run_event_loop()
    {
        while (true)
        {
            if (!handle_epoll_events())
            {
                std::cerr << "   Failed to handle events in EPOLL: " << strerror(errno) << std::endl;
            }
        }
    }

    bool Server::handle_new_client_connection()
    {
        int client_fd;
        while ((client_fd = accept(server_fd_, nullptr, nullptr)) > 0)
        {
            if (!set_non_blocking(client_fd))
                return false;
            add_socket_to_epoll(client_fd, EPOLLIN);
            print_success("Accepted a new connection");
        }
        if (client_fd < 0)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                return false;
            }
        }
        return true;
    }

bool Server::handle_client_data(int client_fd)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    while (true)
    {
        memset(buffer, 0, BUFFER_SIZE);
        std::cout << "Waiting to receive data from client " << client_fd << std::endl;
        bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_received > 0)
        {
            buffer[bytes_received] = '\0';
            std::cout << "Received " << bytes_received << " bytes from client " << client_fd << ": " << buffer << std::endl;
            
            // Send a response back to the client
            const char* response = "Message received\n";
            send(client_fd, response, strlen(response), 0);
            std::cout << "Sent response to client " << client_fd << std::endl;
        }
        else if (bytes_received < 0)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                std::cerr << "Error receiving data from client " << client_fd << ": " << strerror(errno) << std::endl;
                const char* response = "Error receiving data, closing socket";
                send(client_fd, response, strlen(response), 0);
                close(client_fd);
                return false;
            }
            break;
        }
    }
    return true;
}
    bool Server::handle_epoll_events()
    {
        struct epoll_event events[EPOLL_CACHE_SIZE];
        int nfds = epoll_wait(epoll_fd_, events, EPOLL_CACHE_SIZE, -1);
        if (nfds < 0)
        {
            std::cerr << "   Failed to read epoll" << std::endl;
            return false;
        }
        for (int i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == server_fd_)
            {
                if (!handle_new_client_connection())
                {
                    std::cerr << "  Failed to accept connection: " << strerror(errno) << std::endl;
                }
            }
            else
            {
                if (!handle_client_data(events[i].data.fd))
                {
                    std::cerr << "   Failed to handle data " << std::endl;
                }
            }
        }
        return true;
    }

    void Server::print_success(const char *message)
    {
        std::cout << "Success : " << message << "\n"
                  << std::endl;
    }

} // namespace myserver