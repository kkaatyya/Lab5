#include <iostream>
#include <cstring>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>

using namespace std;

#define PORT 8080
#define ROOT_DIR "htmlfiles"

// Функція для надсилання відповіді клієнту
void sendResponse(SOCKET clientSocket, const string& status, const string& body = "")
{
    string response = "HTTP/1.1 " + status + "\r\n" +
                      "Content-Length: " + to_string(body.size()) + "\r\n" +
                      "\r\n" +
                      body;
    int result = send(clientSocket, response.c_str(), response.size(), 0);
    if (result == SOCKET_ERROR)
    {
        cerr << "Send failed" << endl;
    }
}

// Функція для читання вмісту файлу
string readFile(const string& filePath)
{
    ifstream file(filePath, ios::in | ios::binary);
    if (!file)
    {
        return "";
    }
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    return content;
}

// Функція для обробки запиту
void handleRequest(SOCKET clientSocket)
{
    char buffer[1024];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived == SOCKET_ERROR || bytesReceived == 0)
    {
        cerr << "Receive failed or client closed connection" << endl;
        closesocket(clientSocket);
        return;
    }
    buffer[bytesReceived] = '\0';  // Завершуємо рядок

    string request(buffer);

    istringstream requestStream(request);
    string method, path, version;
    requestStream >> method >> path >> version;

    if (method != "GET")
    {
        cerr << "Only GET requests are supported" << endl;
        sendResponse(clientSocket, "400 Bad Request", "Only GET requests are supported.");
        closesocket(clientSocket);
        return;
    }

    if (path != "/index.html" && path != "/page2.html")
    {
        cerr << "Page not found: " << path << endl;
        sendResponse(clientSocket, "404 Not Found", "Page not found.");
        closesocket(clientSocket);
        return;
    }

    string filePath = ROOT_DIR + path;
    string fileContent = readFile(filePath);

    if (fileContent.empty())
    {
        cerr << "File not found: " << filePath << endl;
        sendResponse(clientSocket, "404 Not Found", "File not found.");
    }
    else
    {
        sendResponse(clientSocket, "200 OK", fileContent);
    }

    closesocket(clientSocket);
}


int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket;
    sockaddr_in serverAddr;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        cerr << "Bind failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR)
    {
        cerr << "Listen failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server listening on port " << PORT << "...\n";

    while (true)
    {
        sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET)
        {
            cerr << "Accept failed\n";
            continue;
        }

        char client_addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), client_addr, INET_ADDRSTRLEN);
        cout << "Connection accepted from " << client_addr << ":" << ntohs(clientAddr.sin_port) << endl;

        // Створюємо потік для обробки клієнта
        thread clientThread(handleRequest, clientSocket);
        clientThread.detach();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
