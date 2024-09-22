#include <iostream> // input/output operation
#include <cstring>  //manipualte stream
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <set>
#include <string>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h> // used to gracefully terminate client terminal
#include <pqxx/pqxx>
#include <unordered_map> // hash table for O(1) lookup, VERIFIED client FDs
#define SERVER_PORT 8888
#define PENDING_CONNECTION_BACKLOG 10000
#define EPOLL_CACHE_SIZE 1000000

class DatabaseManager {
private:
    pqxx::connection conn;

public:
    DatabaseManager(const std::string& connString) : conn(connString) {}

    bool verifyUser(const std::string& username, const std::string& password) {
        try {
            pqxx::work txn(conn);
            pqxx::result result = txn.exec(
                "SELECT * FROM users WHERE username = " + txn.quote(username)
            );

            if (result.empty()) {
                return false; // User not found
            }

            // Here you would typically check the password
            // For this example, we're just checking if the user exists
            // In a real application, you'd need to hash the password and compare it

            txn.commit();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error verifying user: " << e.what() << std::endl;
            return false;
        }
    }

    // You can add more methods here for other database operations
};

int server_fd = -1;
int epoll_fd = -1;
const int MAX_SENDERCOMPID = 1000000; // 1 million unique sendercompids, we will use this for an array. Better than hashtable

std::set<int> set_unverifiedfd; // O(log n) for worse case hash collision, this is fine since n is small and cant really build up.

// This array of unordered sets allows for multiple sessions per SenderCompID.
// Each index in the array represents a unique SenderCompID (up to MAX_SENDERCOMPID).
// The unordered_set at each index contains the file descriptors (fds) of verified sessions for that SenderCompID.
// When a new session is verified, its fd is added to the corresponding set.
// When a session ends or becomes inactive, its fd is removed from the set.
// This structure enables efficient management of multiple concurrent sessions for each SenderCompID.
std::array<std::unordered_set<int>, MAX_SENDERCOMPID> array_sendercompid_verifiedfd; 

using namespace std;
namespace arpa_inet
{
    using ::inet_addr;
}
namespace cstring
{
    using ::memset;
}
namespace netinet_in
{
    using ::sockaddr_in;

}
namespace sys_socket
{
    using ::accept;
    using ::bind;
    using ::close;
    using ::listen;
    using ::recv;
    using ::setsockopt;
    using ::socket;
}
namespace sys_epoll
{
    /*
    Create an epoll instance: We'll use epoll_create1() to create an epoll file descriptor.
    Add the server socket to the epoll instance: We'll use epoll_ctl() to add the server socket file descriptor to the epoll instance, monitoring it for incoming connections (EPOLLIN events).
    Implement the main event loop: We'll use epoll_wait() to wait for events on the file descriptors we're monitoring.
    Handle events: When epoll_wait() returns, we'll check what kind of event occurred and handle it accordingly:

    If it's an event on the server socket,
        1. we'll accept the new connection and add the new client socket to the epoll instance.
        2. close socket and remove fd from the epoll instance
    If it's an event on a client socket, we'll read the data from the client.

    struct epoll_event {
        uint32_t events;  // Epoll events, this is bitwise, to combine more than 1 event type we use OR operator
                        // e.g. EPOLLIN | EPOLLOUT
                        // USE & operator to check for a specific event,, we will use if (events[i].events & EPOLLIN)
        epoll_data_t data;  // User data variable
    }
    typedef union epoll_data {
        void *ptr;
        int fd;
        uint32_t u32;
        uint64_t u64;
    } epoll_data_t;

    so we will store the fd inside &epoll_event->data->fd
    */
    using ::epoll_create1;
    using ::epoll_ctl;
    using ::epoll_wait;

}

namespace fix_protocol {

}

void print_success(const char *message)
{
    cout << "Success : " << message << "\n"<< endl;
}
bool add_socket_to_epoll(int *epoll_fd, int *socket_fd, uint32_t events)
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
    event.data.fd = *socket_fd;

    if (sys_epoll::epoll_ctl(*epoll_fd, EPOLL_CTL_ADD, *socket_fd, &event) == -1)
    {
        cerr << "Failed to add socket to epoll" << endl;
        return false;
    }
    return true;
}
bool remove_socket_from_epoll(int *epoll_fd, int *socket_fd)
{
    return epoll_ctl(*epoll_fd, EPOLL_CTL_DEL, *socket_fd, nullptr) != -1;
}

bool bind_and_listen_server_fd(int *server_fd)
{
    cout << "   Binding socket to server IP and port\n   Initializing server_address" << endl;
    struct netinet_in::sockaddr_in server_address;
    cstring::memset(&server_address, 0, sizeof(server_address)); // Setting all bits in that address to 0

    server_address.sin_family = AF_INET; // ipv4
    server_address.sin_addr.s_addr = arpa_inet::inet_addr("127.0.0.1");
    server_address.sin_port = htons(SERVER_PORT); // host byte order to network byte order, not sure why this isnt implicit. also its BYTE. from arpa

    int bind_server_output = sys_socket::bind(*server_fd, (struct sockaddr *)&server_address, sizeof(server_address));
    if (bind_server_output < 0)
    {
        cerr << "Terminating ... failed to bind socket\n    " << strerror(errno) << std::endl;
        sys_socket::close(*server_fd);
        return false;
    }

    print_success("Bound socket to port ");
    // 3. Set the socket to listen for incoming connections
    cout << "   Set Listening to Server socket" << endl;

    if (sys_socket::listen(*server_fd, PENDING_CONNECTION_BACKLOG) < 0)
        cerr << "Terminating ... socket cant listen" << endl;

    print_success("Listening on server fd");
    return true;
}
bool set_non_blocking(int *fd)
{
    /*
    The |= operator is a bitwise OR assignment operator. It adds the O_NONBLOCK flag to the existing flags.
    This means that you’re adding the non-blocking behavior to whatever flags were already set for the file descriptor.

    int fcntl(int fd, int cmd, ... arg)

    */
    int flags = fcntl(*fd, F_GETFL, 0);
    if (flags == -1)
        return false;

    flags |= O_NONBLOCK;

    if (fcntl(*fd, F_SETFL, flags) == -1)
        return false;

    print_success("Success: Set non blocking to fd");
    return true;
}
bool setup_server_and_epoll_fd(int *server_fd, int *epoll_fd)
{
    cout << "\n   Creating file descriptor" << endl;
    *server_fd = sys_socket::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    set_non_blocking(server_fd);
    *epoll_fd = sys_epoll::epoll_create1(0);
    if (*server_fd == -1)
    {
        cerr << "Terminating ... failed to create socket" << endl;
        return false;
    }
    else
    {
        print_success("Created FD for SOCKET SERVER ");
    }
    if (*epoll_fd == -1)
    {
        cerr << "Terminating ... failed to create epoll FD" << endl;
        return false;
    }
    else
    {
        print_success("Created FD for EPOLL ");
    }
    if (!add_socket_to_epoll(epoll_fd, server_fd, EPOLLIN))
    {
        cerr << "Failed to add server FD into EPOLLFD" << endl;
        return false;
    };
    print_success("Added server_fd into epoll_fd");
    bind_and_listen_server_fd(server_fd);
    return true;
}

bool verify_new_client(const char* buffer, size_t buffer_size) {
    // Authentication logic here
    // For now, we'll just assume the client is authenticated
    // We need to check for the username, and password, and check if it exists in the database.
    // we need to use a vector to map the sendercompid to fd
    return true;
}

bool handle_new_client_connection(int *epoll_fd, int *server_fd)
{
    
    int client_fd = sys_socket::accept(*server_fd, nullptr, nullptr);
    while (client_fd > 0)
    {
        if (set_non_blocking(&client_fd) == false)
            return -1;

        // If we successfully accepted a connection, handle the new client
        set_unverifiedfd.insert(client_fd);
        print_success("Accepted a new connection");
        client_fd = sys_socket::accept(*server_fd, nullptr, nullptr); // less than 0 if no new client
    }

    if (client_fd < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // No incoming connection, and since we are non-blocking, this is expected.
            // We simply return and the main loop will call this function again when epoll signals a new event.
            return true; // No new client connection, nothing to do.
        }
        else
        {
            return false; // Return false to indicate failure
        }
        
    }

    return true;
}
bool handle_client_data(int *client_fd)
{
    // 5. Receives bytes
    const int BUFFER_SIZE = 64 * 1024; 
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    while (true)
    {
        cstring::memset(buffer, 0, BUFFER_SIZE);
        bytes_received = sys_socket::recv(*client_fd, buffer, BUFFER_SIZE - 1, 0); // last byte is for null terminator
        
        if (bytes_received > 0)
        {
            buffer[bytes_received] = '\0'; // Null-terminate the received data
            
            if (set_unverifiedfd.count(*client_fd) > 0) {

                if (verify_new_client(buffer, bytes_received) == true) {

                    // TODO get the sendercompid from the buffer
                    set_unverifiedfd.erase(*client_fd);
                    array_sendercompid_verifiedfd[sendercompid].insert(*client_fd);                    
                } else {
                    return false;
                }
            } else {
                // Client is already verified, process the data normally
                cout << "Received " << bytes_received << " bytes: " << buffer << endl;
            }
            return true;
        }
        else if (bytes_received == 0)
        {
            cout << "Received 0 bytes: Removing port" << endl;            
            return false;
        }
        else if (bytes_received < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // No more data to read, continue the loop
                continue;
            }
            else
            {
                cerr << "Error receiving data from client: " << strerror(errno) << endl;
                return false;
            }
        }
    }
}
bool handle_epoll(int *epoll_fd, int *server_fd)
{
    struct epoll_event events[EPOLL_CACHE_SIZE];
    // no. file descriptors =  int epoll_wait(int epoll_fd, struct epoll_event *events, int maxevents, int timeout);
    int nfds = epoll_wait(*epoll_fd, events, EPOLL_CACHE_SIZE, -1);

    if (nfds < 0)
    {
        cerr << "   Failed to read epoll" << endl;
        return false;
    }

    /**
     * @brief When the client closes the connection normally (sending a FIN packet), it will be caught in handle_client_data when recv() returns 0.
    When the client terminates abruptly (like with Ctrl+C), it will be caught in handle_epoll with EPOLLHUP or EPOLLERR events.
     *
     */
    // Iterate through the epoll
    for (int i = 0; i < nfds; i++)
    {
        if (events[i].data.fd == *server_fd) // server_fd wants to establish new client connection

        {
            if (handle_new_client_connection(epoll_fd, server_fd) == false)
            {
                std::cerr << "  Failed to accept connection: " << strerror(errno) << std::endl;
            }
            
        }

        else // client_fd receives new data
        {
            int *client_fd = &(events[i].data.fd);
            
            if (handle_client_data(client_fd) == false)
            {
                // Should also manage Client hung up
                if (!remove_socket_from_epoll(epoll_fd, client_fd))
                {
                    cerr << "failed to remove socket" << endl;
                    return false;
                }
                                
                set_unverifiedfd.erase(*client_fd); // just in case 
                sys_socket::close(*client_fd);
                cout << "Closed socket " << *client_fd << endl;
            }

            
        }
    }
    return true;
}

int main()
{

    // 1. Create FD for server and Epoll + 2. Set up binding + listenx
    if (setup_server_and_epoll_fd(&server_fd, &epoll_fd) == false)
        return -1;

    // 3. Accept and handle incoming connections
    while (true)
    {
        if (handle_epoll(&epoll_fd, &server_fd) == false)
            cerr << "   Failed to handle events in EPOLL - errno : " << strerror(errno) << endl;
    }

    return 0;
}