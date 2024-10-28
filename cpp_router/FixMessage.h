#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <chrono>

class FIXMessage
{
private:
    std::unordered_map<int, std::string> fields; // Map of tags to their corresponding values
    std::vector<int> fieldOrder; // Vector of tags in the order they appear in the message

public:
    FIXMessage(const std::string &message);

    void parse(const std::string &message); // Gotta parse it first before we can read the fields

    std::string getField(int tag) const; // Get the value of a specific tag

    void print() const; // Print the fields in the order they appear

    // Static Method to Create RESPONSE MESSAGES

    static std::string createLogonResponse(const std::string &senderCompId, const std::string &targetCompId);
    static std::string createLogoutResponse(const std::string &senderCompId, const std::string &targetCompId);

};
