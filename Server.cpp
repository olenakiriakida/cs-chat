#include <winsock2.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>

using namespace std;

#define MAX_CLIENTS 10
#define DEFAULT_BUFLEN 4096

#pragma comment(lib, "ws2_32.lib") 
#pragma warning(disable:4996)

SOCKET server_socket;

map<SOCKET, string> client_nicknames;

int main() 
{
    system("title Server");

    puts("Start server... DONE.");
    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) 
    {
        printf("Failed. Error Code: %d", WSAGetLastError());
        return 1;
    }

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) 
    {
        printf("Could not create socket: %d", WSAGetLastError());
        return 2;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) 
    {
        printf("Bind failed with error code: %d", WSAGetLastError());
        return 3;
    }

    listen(server_socket, MAX_CLIENTS);

    puts("Server is waiting for incoming connections...\nPlease, start one or more client-side app.");

    fd_set readfds;
    SOCKET client_socket[MAX_CLIENTS] = {};

    while (true) 
    {
        FD_ZERO(&readfds);

        FD_SET(server_socket, &readfds);

        for (int i = 0; i < MAX_CLIENTS; i++) 
        {
            SOCKET s = client_socket[i];
            if (s > 0) {
                FD_SET(s, &readfds);
            }
        }

        if (select(0, &readfds, NULL, NULL, NULL) == SOCKET_ERROR) 
        {
            printf("select function call failed with error code : %d", WSAGetLastError());
            return 4;
        }

        if (FD_ISSET(server_socket, &readfds)) 
        {
            sockaddr_in address;
            int addrlen = sizeof(sockaddr_in);
            SOCKET new_socket = accept(server_socket, (sockaddr*)&address, &addrlen);

            if (new_socket == INVALID_SOCKET) 
            {
                perror("accept function error");
                return 5;
            }

            printf("New connection, socket fd is %d, ip is: %s, port: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            for (int i = 0; i < MAX_CLIENTS; i++) 
            {
                if (client_socket[i] == 0) 
                {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets at index %d\n", i);

                    const char* join_message = "Новый пользователь присоединился к чату.\n";
                    for (int j = 0; j < MAX_CLIENTS; j++) 
                    {
                        SOCKET clientSocket = client_socket[j];

                        if (clientSocket != 0 && clientSocket != new_socket) 
                        {
                            send(clientSocket, join_message, strlen(join_message), 0);
                        }
                    }

                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) 
        {
            SOCKET s = client_socket[i];

            if (s > 0 && FD_ISSET(s, &readfds)) 
            {
                char client_message[DEFAULT_BUFLEN];
                int client_message_length = recv(s, client_message, DEFAULT_BUFLEN, 0);

                if (client_message_length <= 0) 
                {
                    printf("Client %d disconnected\n", i);

                    string exitMessage = " has left the chat.";

                    for (int j = 0; j < MAX_CLIENTS; j++) 
                    {
                        if (client_socket[j] != 0 && client_socket[j] != s) 
                        {
                            send(s, exitMessage.c_str(), exitMessage.length(), 0);
                        }
                    }

                    closesocket(s);
                    client_socket[i] = 0;
                    client_nicknames.erase(s);

                    continue;
                }
                client_message[client_message_length] = '\0';

                string check_exit = client_message;

                if (check_exit == "off") 
                {
                    cout << "Client #" << i << " is off\n";

                    string exitMessage = " has left the chat.";
                    for (int j = 0; j < MAX_CLIENTS; j++) 
                    {
                        if (client_socket[j] != 0 && client_socket[j] != s) 
                        {
                            send(client_socket[j], exitMessage.c_str(), exitMessage.length(), 0);
                        }
                    }

                    closesocket(s);
                    client_socket[i] = 0;
                    client_nicknames.erase(s);
                }
                else 
                {
                    client_nicknames[s] = client_message;

                    for (int j = 0; j < MAX_CLIENTS; j++) 
                    {
                        if (client_socket[j] != 0 && client_socket[j] != s) 
                        {
                            string message = client_nicknames[s] + ":" + client_message;
                            send(client_socket[j], message.c_str(), client_message_length, 0);
                        }
                    }
                }
            }
        }
    }

    WSACleanup();
}