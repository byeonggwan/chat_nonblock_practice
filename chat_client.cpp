// g++ -o client.exe chat_client.cpp -lws2_32
#include <iostream>
#include <winsock2.h>
#include <string.h>
#include <algorithm>
#include <windows.h>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

SOCKET s;
char nickname[101] = { 0, };

DWORD WINAPI sendFunc(LPVOID lp) {
    char sendData[101] = { 0, };
    char temp[101] = { 0, };

    // Make a form of chatting like below
    // Nickname : chatting
    while (1) {
        memset(sendData, 0, sizeof(sendData));
        strcpy(sendData, nickname);
        strcat(sendData, " : ");
        cin.clear();
        cin.getline(temp, 100);
        strcat(sendData, temp);
        send(s, sendData, strlen(sendData), 0);
    }
}

// For multithreading
/*DWORD WINAPI recvFunc(LPVOID lp) {

    char recvData[101] = {0, };
    while (1) {
        int len = recv(s, recvData, 100, 0);
        if (len > 1)
            cout << recvData << endl;;
    }
}*/

int main() {
    WSADATA wsaData;
    int err;
    u_long nonBlock = 1;
    char temp[101] = { 0, };
    char recvData[101] = { 0, };
    HANDLE sendThread, recvThread;
    DWORD sThreadID, rThreadID;

    // Call WIN SOCKET API
    err = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (err != NO_ERROR) {
        cout << "WSAStartup failed, error code : " << err << endl;
        return -1;
    }

    // Create a socket to connect to just one server
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        cout << "Failed to create a socket" << endl;
        WSACleanup();
        return -1;
    }

    cout << "Connecting to the server..." << endl;

    // connect
    sockaddr_in server;
    int serverLen;
    memset(server.sin_zero, 0, sizeof(server.sin_zero));
    server.sin_family = AF_INET;
    // use your server IP address
    server.sin_addr.s_addr = inet_addr("ser.ver.ip.add");
    server.sin_port = htons(51234);
    serverLen = sizeof(server);

    err = connect(s, (struct sockaddr*)&server, serverLen);
    if (err == SOCKET_ERROR) {
        cout << "Failed to connect to the server" << endl;
        closesocket(s);
        WSACleanup();
        return -1;
    }

    cout << "You can start chatting." << endl;

    // This is just an example code so I don't care about any security
    {
        char sendData[100];
        cout << "Enter your nickname: ";
        cin.getline(nickname, 20);
        strcpy(sendData, "userName=");
        strcat(sendData, nickname);
        send(s, sendData, strlen(sendData), 0);
    }

    /////////////////

    // For non-blocking
    ioctlsocket(s, FIONBIO, &nonBlock);

    sendThread = CreateThread(NULL, 0, sendFunc, NULL, 0, &sThreadID);

    while (1) {
        int len = recv(s, recvData, 100, 0);
        if (len > 1)
            cout << recvData << endl;;
    }

    WaitForSingleObject(sendThread, INFINITE);

    /////////////////

    shutdown(s, SD_SEND);
    closesocket(s);
    WSACleanup();
    return 0;
}