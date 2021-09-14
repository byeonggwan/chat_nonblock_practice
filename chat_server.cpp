#include <iostream>
#include <winsock2.h>
#include <string.h>
#include <algorithm>
#include <stdlib.h>

using namespace std;

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    sockaddr_in myAddr, clientAddr;
    fd_set fs, tempfs;
    int myAddrLen, clientAddrLen;
    int err;
    char recvData[101] = {0, };
    char sendData[101] = {0, };
    timeval timeout;

    // Call WIN SOCKET API
    err = WSAStartup(MAKEWORD(2,2), &wsaData);

    if (err != NO_ERROR) {
        cout << "WSAStartup failed, error code : " << err << endl;
        return -1;
    }

    // Create a socket to connect to just one client
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        cout << "Failed to create a socket" << endl;
        WSACleanup();
        return -1;
    }
    
    cout << "Starting server..." << endl;

    // Binding
    memset(myAddr.sin_zero, 0, sizeof(myAddr.sin_zero));
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myAddr.sin_port = htons(51234);
    myAddrLen = sizeof(myAddr);

    err = bind(s, (struct sockaddr *) &myAddr, myAddrLen);
    if (err == SOCKET_ERROR) {
        cout << "Failed to bind a socket" << endl;
        goto socketError;
    }

    // Listening
    err = listen(s, SOMAXCONN);
    if (err == SOCKET_ERROR) {
        cout << "Failed to listen a socket" << endl;
        goto socketError;
    }

    cout << "Be ready to meet new clients." << endl;
    
    // Initialize FD_SET
    FD_ZERO(&fs);
    FD_SET(s, &fs);
    

    while (1) {
        // It returns 0 when there is no waiting socket
        // Why copy fs? 
        // Answer : select function changes tempfs's contents
        tempfs = fs;
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;

        err = select(0, &tempfs, 0, 0, &timeout);
        if (err < 0) {
            cout << "Failed to select sockets: " << WSAGetLastError() << endl;

            goto socketError;
        }
        if (err == 0) {
            continue;
        }

        // check all socket in fs
        for (auto sock : fs.fd_array) {
            // check sock to be ready to read or not
            if (!FD_ISSET(sock, &tempfs)) {
                continue;
            }
            
            // If sock is a listening socket
            if (sock == s) {
                clientSocket = accept(s, (struct sockaddr *) &clientAddr, &clientAddrLen);
                if (clientSocket == INVALID_SOCKET) {
                    cout << "Failed to proceed with TCP handshake" << endl;
                    goto socketError;
                }

                cout << "new client!!" << endl;
                FD_SET(clientSocket, &fs);
            }

            // If sock is a client socket
            else {
                int recvLen = recv(sock, recvData, 100, 0);
                char temp[100];

                // This is example code so I don't care about any security...
                if (strstr(recvData, "userName=")) {
                    for (int i=9; i<strlen(recvData)+9; i++) {
                        temp[i-9] = recvData[i];
                    }
                    strcat(temp, " entered the room");
                    cout << temp <<endl;
                    for (auto clientSock : fs.fd_array) {
                        if (clientSock == s || clientSock == sock || clientSock == INVALID_SOCKET)
                            continue;
                        send(clientSock, temp, sizeof(temp), 0);
                    }
                    memset(recvData, 0, 100);
                }

                else if (recvLen > 0) {
                    cout << recvData << endl;
                    
                    for (auto clientSock : fs.fd_array) {
                        if (clientSock == s || clientSock == sock || clientSock == INVALID_SOCKET)
                            continue;
                        send(clientSock, recvData, sizeof(recvData), 0);
                    }

                    memset(recvData, 0, sizeof(recvData));
                }

                else {
                    cout << "here" << endl;
                    FD_CLR(sock, &fs);
                    // Very important
                    if (sock != INVALID_SOCKET)
                        closesocket(sock);
                }
            }           
        }        
    }


    cout << "END" << endl;
    shutdown(clientSocket, SD_SEND);
    shutdown(s, SD_SEND);
    closesocket(clientSocket);
    closesocket(s);
    
    WSACleanup();
    return 0;

    socketError:
        closesocket(s);
        WSACleanup();
        system("pause");
        return -1;
}
