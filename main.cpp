#include <iostream>
#include <cstring>
#include <cstdlib>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT 8080
#define HTML_CONTENT "htmlfiles"
void sendResponse(SOCKET clientSocket, const std::string& response)
{
    send(clientSocket, response.c_str(), response.size(), 0);
}
void handleRequest(SOCKET clientSocket)
{
    std::string httpResponse = "HTTP/1.1 200 OK\r\nContent-Length: " +
    std::to_string(strlen(HTML_CONTENT)) + "\r\n\r\n" + HTML_CONTENT;
    sendResponse(clientSocket, httpResponse);
    closesocket(clientSocket);
}
int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET serverSocket;
    struct sockaddr_in serverAddr;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;

    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) ==SOCKET_ERROR)
    {
        std::cerr << "Bind failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    if (listen(serverSocket, 5) == SOCKET_ERROR)
    {
        std::cerr << "Listen failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "Server listening on port " << PORT << "...\n";
    while (true)
    {
        struct sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr,
        &clientAddrLen);
        if (clientSocket == INVALID_SOCKET)
        {
            std::cerr << "Accept failed\n";
            continue;
        }
        char client_addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), client_addr, INET_ADDRSTRLEN);
        std::cout << "Connection accepted from " << client_addr << ":" <<
        ntohs(clientAddr.sin_port) << std::endl;
        handleRequest(clientSocket);
    }
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}