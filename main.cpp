#include <iostream>
#include <cstring>
#include <cstdlib>
#include <string>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>

using namespace std;

#define PORT 8080
#define ROOT_DIR "htmlfiles"

// Функція для надсилання відповіді клієнту
void sendResponse(SOCKET clientSocket, const string& response)
{
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
        cerr << "Recv failed or client closed connection" << endl;
        closesocket(clientSocket);
        return;
    }
    buffer[bytesReceived] = '\0';  // Завершуємо рядок

    string request(buffer);

    // Перевірка чи це GET запит
    if (request.find("GET") == string::npos)
    {
        cerr << "Only GET requests are supported" << endl;
        // Відправляємо помилку 400 для поганих запитів
        string httpResponse = "HTTP/1.1 400 Bad Request\r\n\r\n";
        sendResponse(clientSocket, httpResponse);
        closesocket(clientSocket);
        return;
    }

    // Отримуємо шлях з запиту
    size_t pathStart = request.find("GET ") + 4;
    size_t pathEnd = request.find(" HTTP/");
    string path = request.substr(pathStart, pathEnd - pathStart);

    // Якщо шлях не є /index.html або /page2.html, повертаємо 404
    if (path != "/index.html" && path != "/page2.html")
    {
        cerr << "Page not found: " << path << endl;
        string httpResponse = "HTTP/1.1 404 Not Found\r\n\r\n";
        sendResponse(clientSocket, httpResponse);
        closesocket(clientSocket);
        return;
    }

    // Формуємо шлях до файлу
    string filePath = ROOT_DIR + path;

    // Читаємо вміст файлу
    string fileContent = readFile(filePath);
    if (fileContent.empty())
    {
        cerr << "File not found: " << filePath << endl;
        string httpResponse = "HTTP/1.1 404 Not Found\r\n\r\n";
        sendResponse(clientSocket, httpResponse);
    }
    else
    {
        // Формуємо відповідь з вмістом файлу
        string httpResponse = "HTTP/1.1 200 OK\r\nContent-Length: " +
            to_string(fileContent.size()) + "\r\n\r\n" + fileContent;
        sendResponse(clientSocket, httpResponse);
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
