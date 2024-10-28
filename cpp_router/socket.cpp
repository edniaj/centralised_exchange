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
#include <array>
#include <thread>
#include <sw/redis++/redis++.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <cassert>
#include <iomanip>
#include <chrono>

#define SERVER_PORT 8888
#define PENDING_CONNECTION_BACKLOG 10000
#define EPOLL_CACHE_SIZE 10000

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
    using ::send;
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

#define MAX_SENDERCOMPID 10000 // 1 million unique sendercompids, we will use this for an array. Better than hashtable

class DatabaseManager
{
private:
    pqxx::connection conn;

public:
    DatabaseManager(const std::string &connString) : conn(connString)
    {
        try
        {
            pqxx::work txn(conn);
            pqxx::result result = txn.exec("SELECT COUNT(*) FROM users");
            txn.commit();

            if (!result.empty())
            {
                int userCount = result[0][0].as<int>();
                std::cout << "\n=============================\nDatabase contains " << userCount << " user(s)." << std::endl;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error counting users: " << e.what() << std::endl;
        }
    }

    bool verifyUser(const std::string &username, const std::string &password)
    {
        try
        {
            pqxx::work txn(conn);
            pqxx::result result = txn.exec(
                "SELECT * FROM users WHERE username = " + txn.quote(username));

            if (result.empty())
            {
                return false; // User not found
            }

            // Here you would typically check the password
            // For this example, we're just checking if the user exists
            // In a real application, you'd need to hash the password and compare it

            txn.commit();
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error verifying user: " << e.what() << std::endl;
            return false;
        }
    }

    std::string getUserSenderCompId(const std::string &username)
    {
        try
        {
            pqxx::work txn(conn);
            pqxx::result result = txn.exec(
                "SELECT sendercompid FROM users WHERE username = " + txn.quote(username));

            if (result.empty())
            {
                return ""; // User not found, return empty string
            }

            txn.commit();
            return result[0][0].as<std::string>();
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error fetching user sendercompid: " << e.what() << std::endl;
            return ""; // Return empty string on error
        }
    }

    // You can add more methods here for other database operations
};

class RedisManager
{
private:
    sw::redis::Redis redis;

public:
    RedisManager(const std::string &uri) : redis(uri)
    {
        auto keys = getAllKeys();
        if (keys.empty())
        {
            throw std::runtime_error("Redis database is empty. At least one key should exist.");
        }
        else
        {
            std::cout << "Redis contains " << keys.size() << " key(s).\n=============================\n"
                      << std::endl;
        }
    }

    std::string ping()
    {
        return redis.ping();
    }

    std::vector<std::string> getAllKeys()
    {
        std::vector<std::string> keys;
        redis.keys("*", std::back_inserter(keys));
        return keys;
    }
};

class FIXMessage
{

    // Reference: https://www.fixtrading.org/online-specification/introduction/
private:
    std::unordered_map<int, std::string> fields;
    std::vector<int> fieldOrder;

public:
    FIXMessage(const std::string &message) // Message received must be in a FIX format
    {
        parse(message);
    }

    void parse(const std::string &message)
    {
        size_t pos = 0;
        size_t end = message.length();

        while (pos < end)
        {
            size_t equalPos = message.find('=', pos); // attempt to find '=' from pos
            if (equalPos == std::string::npos)
                break; // npos represents the largest possible value for an element of type size_t, return when not found

            size_t sohPos = message.find('\x01', equalPos); // \x01 is a hexadecimal representation with val 1
            if (sohPos == std::string::npos)
                sohPos = end; // if \x01 not found, \x01 represent end of each tag. sohPos == StartOfHeadingPosition

            int tag = std::stoi(message.substr(pos, equalPos - pos));                // pos to equalPos e.g. "tag"=14
            std::string value = message.substr(equalPos + 1, sohPos - equalPos - 1); // tag="14"

            fields[tag] = value;
            fieldOrder.push_back(tag);

            pos = sohPos + 1;
        }
    }

    std::string getField(int tag) const
    {                                                  // const is a qualifier : doesnt mess with Object state
        auto it = fields.find(tag);                    // automatic type deduction | it is iterator. iterator to a map is a pair object, first : key, second: value
        return (it != fields.end()) ? it->second : ""; // Returns iterator or "" it->second means it.second
    }

    // New method to print the FIXMessage contents
    void print() const
    {
        std::cout << "FIXMessage Contents:" << std::endl;
        for (const auto &tag : fieldOrder)
        {
            std::cout << tag << ":" << fields.at(tag) << std::endl;
        }
    }

    // Add Validation later.

    // Static method to create a logon response
    static std::string createLogonResponse(const std::string &senderCompId, const std::string &targetCompId)
    {
        std::string logonResponse;

        // Begin String
        logonResponse += "8=FIX.4.2\x01";

        // Body Length (placeholder, to be calculated later)
        logonResponse += "9=000000\x01";

        // Message Type (A = Logon)
        logonResponse += "35=A\x01";

        // SenderCompID
        logonResponse += "49=" + senderCompId + "\x01";

        // TargetCompID
        logonResponse += "56=" + targetCompId + "\x01";

        // Message Sequence Number (hardcoded to 1 for this example)
        logonResponse += "34=1\x01";

        // SendingTime (current time in UTC)
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        std::tm *now_tm = std::gmtime(&now_c);
        char timeStr[21];
        std::strftime(timeStr, sizeof(timeStr), "%Y%m%d-%H:%M:%S", now_tm);
        logonResponse += "52=" + std::string(timeStr) + "\x01";

        // EncryptMethod (0 = None/Other)
        logonResponse += "98=0\x01";

        // HeartBtInt (heartbeat interval in seconds, set to 30 for this example)
        logonResponse += "108=30\x01";

        // Calculate and insert the body length
        int bodyLength = logonResponse.length() - 20; // Subtract 20 for the length of tags 8 and 9
        std::string bodyLengthStr = std::to_string(bodyLength);
        logonResponse.replace(logonResponse.find("9=000000") + 2, 6, bodyLengthStr);

        // Calculate and append the CheckSum
        int checkSum = 0;
        for (char c : logonResponse)
        {
            checkSum += static_cast<unsigned char>(c);
        }
        checkSum %= 256;
        char checkSumStr[4];
        std::snprintf(checkSumStr, sizeof(checkSumStr), "%03d", checkSum);
        logonResponse += "10=" + std::string(checkSumStr) + "\x01";

        return logonResponse;
    }
};

void print_success(const char *message)
{
    cout << "Success : " << message << "\n"
         << endl;
}
class TCPServer
{
private:
    int server_fd;
    int epoll_fd;
    DatabaseManager &dbManager;
    std::array<std::unordered_set<int>, MAX_SENDERCOMPID> array_sendercompid_verifiedfd;

    // Private methods (implementation details)
    bool add_socket_to_epoll(int socket_fd, uint32_t events);
    bool remove_socket_from_epoll(int socket_fd);
    bool bind_and_listen();
    bool set_non_blocking(int fd);
    bool handle_new_client_connection();
    bool handle_client_data(int client_fd);
    bool handle_order(int client_fd, FIXMessage &fixMessage);
    bool verify_credential(int client_fd, const FIXMessage &fixMessage);
    bool receive_fix_message(int client_fd, std::string &received_data);

public:
    TCPServer(DatabaseManager &db_manager);

    bool setup();
    void run_login();
    void run_orderbook();
    void run();
    bool sendToClient(int client_fd, const std::string &message);
    bool handle_negative_client_fd(int client_fd);
    int close_client_fd(int client_fd, const char *message);
};

TCPServer::TCPServer(DatabaseManager &db_manager) : dbManager(db_manager), server_fd(-1), epoll_fd(-1) {} // Constructor (parameter) : member initializer list {}

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

bool TCPServer::verify_credential(int client_fd, const FIXMessage &fixMessage)
{
    std::string username = fixMessage.getField(553);
    std::string password = fixMessage.getField(554);

    return dbManager.verifyUser(username, password);
}

bool TCPServer::handle_negative_client_fd(int client_fd)
{
    // No incoming connection, and since we are non-blocking, this is expected.
    // We simply return and the main loop will call this function again when epoll signals a new event.
    // errno is a global variable that is set by the system call that failed.
    if (errno == EAGAIN || errno == EWOULDBLOCK){
        return true; // No new client connection, nothing to do.
        }

    return false; // Return false to indicate failure
}

int TCPServer::close_client_fd(int client_fd, const char *message)
{
    std::cout << message << std::endl;
    sys_socket::close(client_fd);
    return 0;
}

bool TCPServer::whandle_new_client_connection()
{
    int new_client_fd;
    int max_loop = 10000; // NASA STYLE MAX LOOP, we want to avoid infinite loop. Log error if it happens

    // Next time, we will use a loadbalancer to issue the targetCompID
    std::string serverSenderCompID = "SERVER_ASIA_01";


   // Local debug print function
    auto debug_print = [max_loop](const std::string& message) {
        std::cout << "max_loop : " << max_loop <<endl;
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        std::cout << "Debug [" << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << "]: " << message << std::endl;
    };

    do
    {
        // Accept the next client FD
        new_client_fd = sys_socket::accept(server_fd, nullptr, nullptr); // less than 0 if no new client
        if (new_client_fd < 0)                                           // new_client_fd < 0 means no new client or possible error
            return handle_negative_client_fd(new_client_fd);

        // Handle Authentication | Parse the message
        
        std::string received_data;
        if (!receive_fix_message(new_client_fd, received_data))
            return close_client_fd(new_client_fd, "Failed to receive FIX message. Closing connection.");

        // debug_print("Received data: " + received_data);
        // Handle Authentication | Authenticate using fixMessage
        FIXMessage fixMessage(received_data);

        if (!verify_credential(new_client_fd, fixMessage))
            return close_client_fd(new_client_fd, "Failed to verify credentials");
        // debug_print("Verified credentials");

        // Add new socket connection to epoll
        if (!add_socket_to_epoll(new_client_fd, EPOLLIN | EPOLLET))
            return close_client_fd(new_client_fd, "Failed to set non blocking and add to epoll");

        // We need to get the senderCompID from the database
        std::string clientSenderCompID = dbManager.getUserSenderCompId(fixMessage.getField(553));

        // Send logon response
        std::string stringFixMessage = FIXMessage::createLogonResponse(clientSenderCompID, serverSenderCompID);
        if (!sendToClient(new_client_fd, stringFixMessage))
            return close_client_fd(new_client_fd, "Failed to send logon response");

        max_loop--;
    } while (max_loop > 0);

        return handle_negative_client_fd(new_client_fd); // Unwanted error == false, no new client == true
}

bool TCPServer::handle_client_data(int client_fd)
{
    const int BUFFER_SIZE = 1024 * 16;     // 16kb buffer
    std::vector<char> buffer(BUFFER_SIZE); // dynamically sized array, handle malloc dealloc
    ssize_t bytes_received;

    while (true)
    {
        bytes_received = sys_socket::recv(client_fd, buffer.data(), buffer.size(), 0);
        cout << "bytes received: " << bytes_received << endl;
        if (bytes_received > 0)
        {
            buffer[bytes_received] = '\0';                            // Null-terminate the received data
            std::string received_data(buffer.data(), bytes_received); // Convert to std::string so that we can manipulate easier

            FIXMessage fixMessage(received_data);
            fixMessage.print();

            // We need to look at msgtype (field 35 first, some methods might not need login)
            char msgType = fixMessage.getField(35)[0]; // msgtype will always be 1 char

            switch (msgType)
            {
            case 'A':
                return verify_credential(client_fd, fixMessage);
            case 'D':
                return handle_order(client_fd, fixMessage);
            default:
                // Handle other message types or unknown types
                std::cout << "Unhandled message type: " << msgType << std::endl;
                return true;
            }
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
                // No more data to read, exit the loop
                break;
            }
            else
            {
                cerr << "Error receiving data from client: " << strerror(errno) << endl;
                return false;
            }
        }
    }
    return true;
}

bool TCPServer::handle_order(int client_fd, FIXMessage &fixMessage)
{
    // Implement order handling logic here
    return true;
}

void TCPServer::run_login()
{
    cout << "Server is running and accepting connections" << endl;
    while (true)
    {
        cout << "Waiting for events in EPOLL" << endl;
        struct epoll_event events[EPOLL_CACHE_SIZE];
        // no. file descriptors =  int epoll_wait(®int epoll_fd, struct epoll_event *events, int maxevents, int timeout);
        int nfds = epoll_wait(epoll_fd, events, EPOLL_CACHE_SIZE, -1);

        if (nfds < 0)
        {
            cerr << "   Failed to read epoll" << endl;
            continue;
        }

        cout << "Number of events: " << nfds << endl;
        /**
         * @brief When the client closes the connection normally (sending a FIN packet), it will be caught in handle_client_data when recv() returns 0.
        When the client terminates abruptly (like with Ctrl+C), it will be caught in handle_epoll with EPOLLHUP or EPOLLERR events.
         *
         */
        // Iterate through the epoll
        for (int i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == server_fd) // server_fd wants to establish new client connection
            {
                if (handle_new_client_connection() == false)
                {
                    std::cerr << "  Failed to accept connection: " << strerror(errno) << std::endl;
                }
            }
            else // client_fd receives new data
            {
                int client_fd = events[i].data.fd;

                if (handle_client_data(client_fd) == false)
                {
                    if (!remove_socket_from_epoll(client_fd))
                    {
                        cerr << "failed to remove socket" << endl;
                        continue;
                    }

                    sys_socket::close(client_fd);
                    cout << "Closed socket " << client_fd << endl;
                }
            }
        }
        cout << "done" << endl;
    }
}

void TCPServer::run_orderbook()
{
    cout << "running orderbook" << endl;
}

void TCPServer::run()
{
    std::thread login_thread(&TCPServer::run_login, this);
    std::thread orderbook_thread(&TCPServer::run_orderbook, this);

    login_thread.join();
    orderbook_thread.join();
}

bool TCPServer::receive_fix_message(int client_fd, std::string &received_data)
{
    const int BUFFER_SIZE = 1024 * 16; // 16kb buffer
    std::vector<char> buffer(BUFFER_SIZE);
    ssize_t bytes_received;

    received_data.clear();

    while (true)
    {
        bytes_received = sys_socket::recv(client_fd, buffer.data(), buffer.size(), 0);
        if (bytes_received > 0)
        {
            received_data.append(buffer.data(), bytes_received);
            if (received_data.find("\x01") != std::string::npos) // FIX messages end with SOH character
            {
                return true;
            }
        }
        else if (bytes_received == 0 || (bytes_received < 0 && errno != EAGAIN && errno != EWOULDBLOCK))
        {
            return false;
        }
        else
        {
            break; // No more data to read for now
        }
    }

    return !received_data.empty();
}

int main()
{

    // Used for verification
    DatabaseManager dbManager("dbname=docker user=docker password=docker host=localhost");
    // Instantiate RedisManager
    RedisManager redisManager("tcp://127.0.0.1:6379");

    TCPServer server(dbManager);

    if (!server.setup())
    {
        return -1;
    }

    server.run();

    return 0;
}

/*
 * CRITICAL ISSUES:
 *
 * 1. EPOLL Timestamp Queue:
 *    The current implementation of EPOLL does not put edited file descriptors
 *    into a queue system based on timestamp. This can lead to potential issues
 *    with handling events in the order they occurred, especially in high-load
 *    scenarios. Implementing a timestamp-based queue for EPOLL events would
 *    ensure that we process events in the correct order and prevent any
 *    race conditions or out-of-order processing.
 *
 * 2. Separate EPOLLs for Login and Orderbook:
 *    Currently, we are using a single EPOLL instance for both login and
 *    orderbook operations. This design can lead to performance bottlenecks
 *    and make it difficult to manage different types of events efficiently.
 *    We should implement two separate EPOLL instances:
 *    - One for handling login-related events
 *    - Another for managing orderbook-related events
 *    This separation would allow for better scalability, improved performance,
 *    and easier management of different types of operations within the system.
 */


