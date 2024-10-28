#include "FixMessage.h"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>


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
