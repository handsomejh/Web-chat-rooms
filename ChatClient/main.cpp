#include<WinSock2.h>
#include<WS2tcpip.h>
#include<Windows.h>
#include<iostream>
#include<stdio.h>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#define BUFF_SIZE 1024

char szMsg[BUFF_SIZE];

// 发消息
unsigned SendMsg(void* arg) {
    SOCKET sock = *((SOCKET*)arg);
    while (1) {
        scanf("%s", szMsg);
        if (strcmp(szMsg, "QUIT") == 0 || strcmp(szMsg, "quit") == 0) {
            closesocket(sock);
            exit(0);
        }

        send(sock, szMsg, strlen(szMsg), 0);
    }
    return 0;
}

// 接收消息
unsigned RecvMsg(void* arg) {
    SOCKET sock = *((SOCKET*)arg);
    char msg[BUFF_SIZE];
    while (1) {
        int len = recv(sock, msg, sizeof(msg) - 1, 0);
        if (len == -1) {
            return -1;
        }
        msg[len] = '\0';
        cout << msg << endl;
    }
    return 0;
}

int main() {
    // 初始化socket环境
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        return -1;
    }

    if (LOBYTE(wsaData.wVersion) != 2 ||
        HIBYTE(wsaData.wVersion) != 2) {
        WSACleanup();
        return -1;
    }

    // 创建socket

    SOCKET hSock;
    hSock = socket(AF_INET, SOCK_STREAM, 0);

    // 绑定端口
    SOCKADDR_IN servAdr;
    memset(&servAdr, 0, sizeof(servAdr));
    servAdr.sin_family = AF_INET;
    servAdr.sin_port = htons(9999);
    inet_pton(AF_INET, "150.158.138.90", &servAdr.sin_addr);
    
    //连接服务器
    if (connect(hSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR) {
        cout << "connect error : " << WSAGetLastError() << endl;
        closesocket(hSock);
        WSACleanup();
        return -1;
    }
    else {
        cout << "欢迎进入聊天室，请输入你的聊天用户名：";
    }

    // 循环发消息
    HANDLE hSendHand = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendMsg, (void*)&hSock, 0, NULL);

    // 循环收消息
    HANDLE hRecvHand = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecvMsg, (void*)&hSock, 0, NULL);

    // 等待线程结束
    WaitForSingleObject(hSendHand, INFINITE);
    WaitForSingleObject(hRecvHand, INFINITE);

    closesocket(hSock);
    WSACleanup();
}