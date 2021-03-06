// winsocket_server.cpp: 定義主控台應用程式的進入點。
//
#include "stdafx.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>

using namespace std;

#define MAX_CONNECTIONS 10 //最大連接個數

int MaxSocket = 0;
#define MY_PORT_NUMBER 1234

#pragma comment(lib, "Ws2_32.lib")//鏈接Ws2_32.lib這個Library


DWORD WINAPI RunForClientThread(LPVOID);
SOCKET connections[MAX_CONNECTIONS];

vector<int>  online_list;
void del_Element_From_Online_list_Vec(int who); //找出 指定的元素內容 並從 vector 移除
void transfer(string msg, int ID);//寄送訊息給其他 上線的client,並標註自己的ID

int main()
{
	DWORD ThreadID;
	HANDLE ThreadHandle[MAX_CONNECTIONS];
	//用WSAStartup開始Winsock-DLL
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 1), &wsaData); 
	//MAKEWORD(2, 1)為Winsocket-DLL版本
	//對Sockets 初始化,成功以後才能進一步使用Windows Sockets API
	
	//宣告socket位址資訊
	struct sockaddr_in addr;
	int addrLen = sizeof(addr);

	//建立socket
	SOCKET sConnect;
	sConnect = socket(AF_INET, SOCK_STREAM, NULL);
	
	//設定位址資訊的資料
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(MY_PORT_NUMBER);

	//設定監聽Listen Socket
	SOCKET sListen;
	sListen = socket(AF_INET, SOCK_STREAM, NULL);
	bind(sListen, (sockaddr*)&addr, addrLen);
	listen(sListen, SOMAXCONN);

	//宣告clientAddr儲存client的位址資訊
	SOCKADDR_IN clientAddr;
	printf("Server starting...\n");

	while (true)
	{
		//等待client連線
		if (sConnect = accept(sListen, (SOCKADDR*)&clientAddr, &addrLen))
		{
			printf("a connection was found!!\n");
			
			//檢查是否有未建立連線的Socket可用
			int sokcet_index = -1;
			for (int i=0; i<MAX_CONNECTIONS; i++)
			{
				if (connections[i] == 0)
				{
					sokcet_index = i;
					MaxSocket = i;
					break;
				}
			}
			if (sokcet_index == -1)
			{
				printf("Connection full... \n");
				return 1;
			}
			connections[sokcet_index] = sConnect;
			
			ThreadHandle[MaxSocket] = 
				CreateThread(NULL, 0, RunForClientThread, &sokcet_index, 0, &ThreadID);
		}
		
	}

	for(int i=0;  i<MAX_CONNECTIONS;i++)
		closesocket(connections[i]);
	WSACleanup();
    return 0;
}

DWORD WINAPI RunForClientThread(LPVOID input_sIndex) {

	string otherClientString = ""; //從client端收到的message
	char sendbuf[200], recvbuf[200];
	int sockIndex = *(int*)input_sIndex;
	string str;//任何的輸入
	int recvReturn;

	string ID = to_string(sockIndex);//紀錄ID
	online_list.push_back(sockIndex);//更新上線名單

	cout << "服務 " + ID + " Thread start!"<< endl;


	//傳送訊息給 client 端
	strcpy_s(sendbuf, "Hello client!! ");//準備字串
	str = sendbuf + '\0';
	send(connections[sockIndex], str.c_str(), (int)strlen(str.c_str()), 0);//寄送 歡迎訊息
	send(connections[sockIndex], ID.c_str(), (int)strlen(ID.c_str()), 0);//寄送 ID = sockIndex
	
	ZeroMemory(recvbuf, sizeof(recvbuf));
	while (true) {//接收 client 端的訊息, 在client 還沒發出 exit 之前都不會結束
		ZeroMemory(recvbuf, sizeof(recvbuf));
		if ( ( recvReturn =  recv(connections[sockIndex], recvbuf, sizeof(recvbuf), 0)) > 0 ) 
											//recv() = 0 ,代表遠端那邊已經關閉了你的連線, < 0 : 表示錯誤
		{
			str.clear();
			cout << "recvReturn : " << recvReturn << endl;
			str = to_string(sockIndex) + " : ";
			str += recvbuf;
			transfer(str, atoi(ID.c_str()) );
		}
		else {
			str = ID + "  Disconnected!";
			cout << str << endl;
			transfer(str, atoi(ID.c_str()));//告訴其他使用者 誰斷開
			closesocket(connections[atoi(ID.c_str())]);//斷開連結!
			connections[atoi(ID.c_str())] = 0; //讓這 socket 閒置出來
			del_Element_From_Online_list_Vec(sockIndex);//移除使用者 從 online_client 名單
			return 0;
		}
	}//while end

	return 0;
}

void del_Element_From_Online_list_Vec(int who) { //找出 指定的元素內容 並從 vector 移除
	for (int i = 0; i < online_list.size(); i++) {//走訪 vector
		if (online_list[i] == who) {
			swap(online_list[i], online_list[online_list.size() - 1]);// 將目標移至vector尾端
			online_list.pop_back();//pop() 出來
		}
	}
}

void transfer(string msg, int ID) {
	char sendMsg[200];
	string title = "0 : ";
	if (msg.size() == title.size()) {//這邊的msg 已經加上 title -> "0 : ",我們不應該只寄出標頭 至少會有訊息
		return;
	}
	cout << "轉送的訊息為 >> " << msg << "<<"<<  endl;
	for (int i = 0; i < online_list.size(); i++) {//走訪 online_client 所有元素
		if (online_list[i] == ID) {//當發現寄送者是自己時
			continue; //換下一個
		}
		else {
			ZeroMemory(sendMsg, 200);
			strcpy_s(sendMsg, msg.c_str());
			send(connections[online_list[i]], sendMsg, (int)strlen(sendMsg), 0);//寄送 msg 訊息	
		}
	}
return;
}
