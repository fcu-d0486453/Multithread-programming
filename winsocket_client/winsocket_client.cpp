// winsocket_client.cpp: 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#pragma comment(lib, "Ws2_32.lib")

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include <windows.h>

#define MY_PORT_NUMBER 1234

using namespace std;

void sentMsgToServer(SOCKET, string );

int MY_ID; //my_ID 的全域變數版本

DWORD WINAPI RunForServerThread(LPVOID);//+++

SOCKET connections;

void main()
{
	char confirm;
	char recvMsg[200];
	static int my_ID;//等連到 Server 才會給
	string str;//使用者要輸入的訊息
	//開始 Winsock-DLL
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 1), &wsaData);
	
	
	//宣告給 socket 使用的 sockadder_in 結構
	SOCKADDR_IN addr;
	int addlen = sizeof(addr);

	
	//設定 socket
	SOCKET sConnect;
	sConnect = socket(AF_INET, SOCK_STREAM, NULL);
	connections = sConnect;


	//設定欲連線的Server的位址資訊
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(MY_PORT_NUMBER);
	
	printf("connect to server?[Y] or [N] --> ");
	scanf_s("%c", &confirm,1);
	cin.get();//防止 sentMsgToServer 吃Y
	
	Sleep(1000);
	if (confirm == 'N')
	{
		exit(1);
	}
	else {
		if (confirm == 'Y')
		{
			connect(sConnect, (SOCKADDR*)&addr, sizeof(addr));
			
			DWORD ThreadID;
			HANDLE ThreadHandleForListen;//拿來監聽 之後 Server 的回傳

			//接收 server 端的訊息 這邊直接重複收兩次  1.msg、2.ID
			ZeroMemory(recvMsg, 200);
			recv(sConnect, recvMsg, sizeof(recvMsg), 0);//從 sever 接收
			printf("Server-> %s \n", recvMsg); //client 端印出
			recv(sConnect, recvMsg, sizeof(recvMsg), 0);//從 sever 接收 ID 
			my_ID = atoi(recvMsg);
			printf("Your Id: %d\n", my_ID); //client 端印出*/
			MY_ID = my_ID;//給全域變數
			
			cout << "Type in \"exit\" to exit client!" << endl;

			ThreadHandleForListen = //監聽Server 的 thread 開始執行
				CreateThread(NULL, 0, RunForServerThread, NULL, 0, &ThreadID);
			CloseHandle(ThreadHandleForListen);
			//main 沒用到 handle ,就關閉 但這不會終止 thread.

			while (true) {//收、送資料 迴圈
				
				cout << to_string(my_ID) + " -> ";
				getline(cin, str, '\n');
				if (str == "exit") { //輸入 exit 終止 client連接
					closesocket(sConnect);
					break; 
				}
				else {
					if (str.length() != 0) {
						
						sentMsgToServer(sConnect, str);
						str = "";//初始化
					}else	continue;
				}
			}
		}
	}
	//closesocket(sConnect);
	WSACleanup();
	printf("bye bye...\n");
	system("pause");
	return;
}
void sentMsgToServer(SOCKET sConnect,string str) {
	char sendMsg[200];
	str += '\n';
	ZeroMemory(sendMsg, 200);
	strcpy_s(sendMsg, str.c_str());
	send(sConnect, sendMsg, (int)strlen(sendMsg), 0);
	//ZeroMemory(sendMsg, 200);
	return;
}

DWORD WINAPI RunForServerThread(LPVOID input_sIndex) {//用來監聽 Sever 的轉傳
	char recvMsg[200];
	
	ZeroMemory(recvMsg, 200);
	while (true) {
		if (recv(connections, recvMsg, sizeof(recvMsg), 0) > 0)//從 sever 接收 
															 //recv() = 0 ,代表遠端那邊已經關閉了你的連線
		{
			//因為我的recvMsg 在某種情況下會被加上 殘留字元,導致  "%s" 印錯.
			/*
		  來源>>	|正常字串\n\0
			 	|&^$&^%&\n......
				=============
				擷取第一個\n前的字就好
			*/
			char clrstr[200],*useless;
			strcpy_s(clrstr,recvMsg);//把 recvMsg(指標變數) -> clrstr(指標常數)
			char *ptr = strtok_s(clrstr, "\n", &useless);//第一的參數要是指標常數
			printf("來自Server 的傳送> %s<\n", ptr);
			cout << to_string(MY_ID) << " -> ";
		}
		else {
			ZeroMemory(recvMsg, 200);
		}
	}
	return 0;
}
