#include <iostream>
#include <cstring>
#include <cstdlib>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>

using namespace std;

#define PORT 8080
#define ROOT_DIR "htmlfiles"

void sendResponse(SOCKET clientSocket, const string& response)
{
    int result = send(clientSocket, response.c_str(), response.size(), 0);
    if (result == SOCKET_ERROR)
    {
        cerr << "Send failed" << endl;
    }
}

void handleRequest(SOCKET clientSocket)
{
    // Читання запиту клієнта
    char buffer[1024];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived == SOCKET_ERROR || bytesReceived == 0)
    {
        cerr << "Recv failed or client closed connection" << endl;
        closesocket(clientSocket);
        return;
    }
    buffer[bytesReceived] = '\0'; // Завершуємо рядок

    string request(buffer);

    // Перевірка чи GET запит
    if (request.find("GET") == string::npos)
    {
        cerr << "Only GET requests are supported" << endl;
        closesocket(clientSocket);
        return;
    }

    // Формуємо відповідь
    string httpResponse = "HTTP/1.1 200 OK\r\nContent-Length: " +
        to_string(strlen(ROOT_DIR)) + "\r\n\r\n" + ROOT_DIR;

    sendResponse(clientSocket, httpResponse);
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
