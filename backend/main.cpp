#include <iostream>
#include <string>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// ================= RECURSIVE FUNCTIONS =================

// 1. Sum of digits
int sumDigits(int n)
{
    if (n < 10)
        return n;

    return (n % 10) + sumDigits(n / 10);
}

// 2. Reverse number
int reverseHelper(int n, int result)
{
    if (n == 0)
        return result;

    return reverseHelper(n / 10, result * 10 + (n % 10));
}

int reverseNumber(int n)
{
    return reverseHelper(n, 0);
}

// 3. Count digits
int countDigits(int n)
{
    if (n < 10)
        return 1;

    return 1 + countDigits(n / 10);
}

// 4. Palindrome
bool isPalindrome(int n)
{
    return n == reverseNumber(n);
}

// 5. Factorial
long long factorial(int n)
{
    if (n <= 1)
        return 1;

    return n * factorial(n - 1);
}

// 6. Power
long long power(int base, int exponent)
{
    if (exponent == 0)
        return 1;

    return base * power(base, exponent - 1);
}

// 7. Fibonacci
long long fibonacci(int n)
{
    if (n <= 1)
        return n;

    return fibonacci(n - 1) + fibonacci(n - 2);
}


// ================= JSON HELPERS =================

string getValue(string json, string key)
{
    string search = "\"" + key + "\"";

    size_t pos = json.find(search);

    if (pos == string::npos)
        return "";

    pos = json.find(":", pos);

    if (pos == string::npos)
        return "";

    pos++;

    while (pos < json.length() && json[pos] == ' ')
        pos++;

    // String value
    if (json[pos] == '"')
    {
        pos++;

        size_t end = json.find('"', pos);

        return json.substr(pos, end - pos);
    }

    // Number value
    size_t end = json.find_first_of(",}", pos);

    return json.substr(pos, end - pos);
}


// ================= HTTP RESPONSE =================

void sendResponse(SOCKET client, string response)
{
    string httpResponse =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Content-Length: " + to_string(response.length()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" +
        response;

    send(client, httpResponse.c_str(),
         httpResponse.length(), 0);
}


// ================= CLIENT HANDLER =================

void handleClient(SOCKET client)
{
    char buffer[8192];

    int bytesReceived = recv(
        client,
        buffer,
        sizeof(buffer) - 1,
        0
    );

    if (bytesReceived <= 0)
    {
        closesocket(client);
        return;
    }

    buffer[bytesReceived] = '\0';

    string request(buffer);

    // OPTIONS request
    if (request.find("OPTIONS") == 0)
    {
        string response =
            "HTTP/1.1 204 No Content\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

        send(
            client,
            response.c_str(),
            response.length(),
            0
        );

        closesocket(client);
        return;
    }

    // Only POST /calculate
    if (request.find("POST /calculate") == 0)
    {
        size_t bodyStart = request.find("\r\n\r\n");

        if (bodyStart == string::npos)
        {
            sendResponse(
                client,
                "{\"error\":\"Invalid request\"}"
            );

            closesocket(client);
            return;
        }

        string body =
            request.substr(bodyStart + 4);

        string numberText =
            getValue(body, "number");

        string operation =
            getValue(body, "operation");

        int number =
            stoi(numberText);

        string result;

        // ================= OPERATIONS =================

        if (operation == "sum")
        {
            result =
                "{\"result\":" +
                to_string(sumDigits(number)) +
                "}";
        }

        else if (operation == "reverse")
        {
            result =
                "{\"result\":" +
                to_string(reverseNumber(number)) +
                "}";
        }

        else if (operation == "count")
        {
            result =
                "{\"result\":" +
                to_string(countDigits(number)) +
                "}";
        }

        else if (operation == "palindrome")
        {
            bool answer =
                isPalindrome(number);

            result =
                "{\"result\":\"" +
                string(answer ? "Palindrome" : "Not Palindrome") +
                "\"}";
        }

        else if (operation == "factorial")
        {
            if (number < 0)
            {
                result =
                    "{\"error\":\"Factorial is not defined for negative numbers\"}";
            }
            else
            {
                result =
                    "{\"result\":" +
                    to_string(factorial(number)) +
                    "}";
            }
        }

        else if (operation == "power")
        {
            // Example: number = 5
            // Calculates 5^2
            result =
                "{\"result\":" +
                to_string(power(number, 2)) +
                "}";
        }

        else if (operation == "fibonacci")
        {
            if (number < 0)
            {
                result =
                    "{\"error\":\"Fibonacci requires a non-negative number\"}";
            }
            else
            {
                result =
                    "{\"result\":" +
                    to_string(fibonacci(number)) +
                    "}";
            }
        }

        else
        {
            result =
                "{\"error\":\"Unknown operation\"}";
        }

        sendResponse(client, result);
    }
    else
    {
        sendResponse(
            client,
            "{\"error\":\"Use POST /calculate\"}"
        );
    }

    closesocket(client);
}


// ================= MAIN =================

int main()
{
    WSADATA wsaData;

    // Start Windows Socket
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "WSAStartup failed!" << endl;
        return 1;
    }

    SOCKET serverSocket =
        socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == INVALID_SOCKET)
    {
        cout << "Socket creation failed!" << endl;

        WSACleanup();

        return 1;
    }

    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);

    if (bind(
            serverSocket,
            (sockaddr*)&serverAddress,
            sizeof(serverAddress)
        ) == SOCKET_ERROR)
    {
        cout << "Bind failed!" << endl;

        closesocket(serverSocket);
        WSACleanup();

        return 1;
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR)
    {
        cout << "Listen failed!" << endl;

        closesocket(serverSocket);
        WSACleanup();

        return 1;
    }

    cout << "==================================" << endl;
    cout << " Recursive Number Explorer" << endl;
    cout << " Backend Server Started!" << endl;
    cout << " http://localhost:8080" << endl;
    cout << "==================================" << endl;

    while (true)
    {
        SOCKET client =
            accept(serverSocket, nullptr, nullptr);

        if (client == INVALID_SOCKET)
        {
            cout << "Client connection failed!" << endl;
            continue;
        }

        handleClient(client);
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}