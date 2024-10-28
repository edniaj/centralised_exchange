bool TCPServer::add_socket_to_epoll(int socket_fd, uint32_t events)
{
    /**

    epoll_ctl is a function used to control epoll. It takes four arguments:
    epoll_fd: The file descriptor of the epoll instance.
    EPOLL_CTL_ADD: The operation (in this case, we're adding a file descriptor).
    socket_fd: The file descriptor we're adding to epoll.
    &event: A pointer to the epoll_event structure.
     *
     */
    struct epoll_event event;
    event.events = events;
    event.data.fd = socket_fd;
    if (set_non_blocking(socket_fd) == false)
        return false;
    if (sys_epoll::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == -1)
    {
        cerr << "Failed to add socket to epoll" << endl;
        return false;
    }
    return true;
}
bool TCPServer::sendToClient(int client_fd, const std::string &message)
{
    ssize_t total_sent = 0;
    ssize_t message_length = message.length();
    const char *buffer = message.c_str();

    while (total_sent < message_length)
    {
        ssize_t sent = send(client_fd, buffer + total_sent, message_length - total_sent, 0);
        if (sent == -1)
        {
            if (errno == EINTR)
            {
                // Interrupted by signal, retry
                continue;
            }
            else if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // Socket buffer is full, wait and retry
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            else
            {
                // Unrecoverable error
                std::cerr << "Error sending message to client: " << strerror(errno) << std::endl;
                return false;
            }
        }
        total_sent += sent;
    }

    return true;
}
bool TCPServer::remove_socket_from_epoll(int socket_fd)
{
    return epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket_fd, nullptr) != -1;
}

bool TCPServer::bind_and_listen()
{
    cout << "   Binding socket to server IP and port\n   Initializing server_address" << endl;
    struct netinet_in::sockaddr_in server_address;
    cstring::memset(&server_address, 0, sizeof(server_address)); // Setting all bits in that address to 0

    server_address.sin_family = AF_INET; // ipv4
    server_address.sin_addr.s_addr = arpa_inet::inet_addr("127.0.0.1");
    server_address.sin_port = htons(SERVER_PORT); // host byte order to network byte order, not sure why this isnt implicit. also its BYTE. from arpa

    int bind_server_output = sys_socket::bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address));
    if (bind_server_output < 0)
    {
        cerr << "Terminating ... failed to bind socket\n    " << strerror(errno) << std::endl;
        sys_socket::close(server_fd);
        return false;
    }

    print_success("Bound socket to port ");
    // 3. Set the socket to listen for incoming connections
    cout << "   Set Listening to Server socket" << endl;

    if (sys_socket::listen(server_fd, PENDING_CONNECTION_BACKLOG) < 0)
        cerr << "Terminating ... socket cant listen" << endl;

    print_success("Listening on server fd");
    return true;
}

bool TCPServer::set_non_blocking(int fd)
{
    /*
    The |= operator is a bitwise OR assignment operator. It adds the O_NONBLOCK flag to the existing flags.
    This means that you're adding the non-blocking behavior to whatever flags were already set for the file descriptor.

    int fcntl(int fd, int cmd, ... arg)

    */
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return false;

    flags |= O_NONBLOCK;

    if (fcntl(fd, F_SETFL, flags) == -1)
        return false;

    print_success("Success: Set non blocking to fd");
    return true;
}

bool TCPServer::setup()
{
    cout << "\n   Creating file descriptor" << endl;
    server_fd = sys_socket::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    set_non_blocking(server_fd);
    epoll_fd = sys_epoll::epoll_create1(0);
    if (server_fd == -1)
    {
        cerr << "Terminating ... failed to create socket" << endl;
        return false;
    }
    else
    {
        print_success("Created FD for SOCKET SERVER ");
    }
    if (epoll_fd == -1)
    {
        cerr << "Terminating ... failed to create epoll FD" << endl;
        return false;
    }
    else
    {
        print_success("Created FD for EPOLL ");
    }
    if (!add_socket_to_epoll(server_fd, EPOLLIN))
    {
        cerr << "Failed to add server FD into EPOLLFD" << endl;
        return false;
    };
    print_success("Added server_fd into epoll_fd");
    bind_and_listen();
    return true;
}
