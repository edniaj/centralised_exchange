#include <iostream> // input/output operation
#include <cstring>  //manipualte stream
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
using namespace std;

namespace sys_socket
{
    using ::accept;
    using ::bind;
    using ::close;
    using ::listen;
    using ::setsockopt;
    using ::socket;
}

namespace netinet_in
{
    using ::sockaddr_in;

}

namespace arpa_inet
{
    using ::inet_addr;
}

namespace cstring
{
    using ::memset;
}
const int SERVER_PORT = 8888;

void print_success(const char *message)
{
    cout << "Success : " << message << "\n"
         << endl;
}

int main()
{
    struct netinet_in::sockaddr_in server_address;
    int server_fd;

    // 1. Create FD for server socket
    cout << "\n   Creating file descriptor" << endl;
    server_fd = sys_socket::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == -1)
    {
        cerr << "Terminating ... failed to create socket" << endl;
        return -1;
    }
    else
    {
        print_success("Created FD for SOCKET SERVER ");
    }

    // 2. Bind the socket to the correct IP:SERVER_PORT
    cout << "   Binding socket to server IP and port" << endl;

    // Initialize struct sockaddr_in address
    cout << "   Initializing server_address" << endl;

    cstring::memset(&server_address, 0, sizeof(server_address)); // Setting all bits in that address to 0

    server_address.sin_family = AF_INET; // ipv4
    server_address.sin_addr.s_addr = arpa_inet::inet_addr("127.0.0.1");
    server_address.sin_port = htons(SERVER_PORT); // host byte order to network byte order, not sure why this isnt implicit. also its BYTE. from arpa

    // perform the bind between the FD and the server_address
    int bind_server_output = sys_socket::bind(server_fd,
                                              (struct sockaddr *)&server_address, sizeof(server_address));
    if (bind_server_output < 0)
    {
        cerr << "Terminating ... failed to bind socket" << endl;
        sys_socket::close(server_fd);
        return -1;
    }
    else
    {
        print_success("Bound socket to port ");
    }

    // 3. Set the socket to listen for incoming connections
    cout << "   Set Listening to Server socket" << endl;
    int backlog = 10000; // queue size for pending connection waiting to be accept()
    if (sys_socket::listen(server_fd, backlog) < 0)
    {
        cerr << "" << endl;
    }
    else
    {
        print_success("Listening on server fd");
    };

    // 4. Accept and handle incoming connections
    while (true)
    {
        int client_fd = sys_socket::accept(server_fd, nullptr, nullptr); // arg(fd, socket_addr_in, sizeof(socket_addr_in)) the struct that we want to store the client address in

        if (client_fd < 0)
        {
            cerr << "Failed to accept connection" << endl;
            continue;
        }

        print_success("Accepted a connection");

        // 5. Receives bytes
        const int BUFFER_SIZE = 64 * 1024; // 64 kb buffer
        char buffer[BUFFER_SIZE];
        ssize_t bytes_received;
        while (true)
        {
            memset(buffer, 0, BUFFER_SIZE);
            bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0); // last byte is for null terminator

            if (bytes_received < 0)
            {
                cerr << "Error receiving data from client" << endl;
                break;
            }
            else
            {
                buffer[bytes_received] = '\0'; // Null-terminate the received data
                cout << "Received " << bytes_received << " bytes: " << buffer << endl;

            }
        }
        sys_socket::close(client_fd); // Close the client socket when done
    }

    return 0;
}
