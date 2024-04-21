#include <winsock2.h>
#include <iostream>
#include <string>

using namespace std;

#define DEFAULT_BUFLEN 4096

#pragma comment(lib, "ws2_32.lib") 
#pragma warning(disable:4996)

int main() 
{
    system("title Client");

    puts("Start client... DONE.");

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }

    SOCKET client_socket;

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) 
    {
        printf("Could not create socket: %d", WSAGetLastError());
        return 2;
    }

    sockaddr_in server_addr;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8888);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) 
    {
        printf("Connection error");
        return 3;
    }

    puts("Connected!");

    char nickname[DEFAULT_BUFLEN];
    cout << "Enter your nickname: ";
    cin.getline(nickname, DEFAULT_BUFLEN);
    send(client_socket, nickname, strlen(nickname), 0);

    while (true) 
    {
        char message[DEFAULT_BUFLEN];
        cout << "You: ";
        cin.getline(message, DEFAULT_BUFLEN);

        if (strcmp(message, "exit") == 0) 
        {
            send(client_socket, "off", strlen("off"), 0);
            break;
        }

        send(client_socket, message, strlen(message), 0);
    }

    closesocket(client_socket);
    WSACleanup();

    return 0;
}