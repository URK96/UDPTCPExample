
#define FILEPATHMAXLENGTH 255
#define BUFFERSIZE 1024

#include <cstdlib>
#include <iostream>
#include <WinSock2.h>

#pragma comment (lib, "ws2_32.lib")
#pragma warning(disable: 4996)

using namespace std;

typedef enum Type { Server, Client }Type;

unsigned short serverPort = 0;
unsigned short clientPort = 0;

void GetPortNum(Type type, unsigned short min, unsigned short max)
{
	while (1)
	{
		int portNum = 0;

		cout << "Input " << type << " port number (" << min << " ~ " << max << ") => ";
		cin >> portNum;

		if ((portNum >= min) && (portNum <= max))
		{
			if (type == Server)
				serverPort = portNum;
			else
				clientPort = portNum;

			break;
		}
		else
		{
			cout << "Invalid " << type << " port number. Please input again.";
			Sleep(1000);
			system("cls");
		}
	}
}

int MainMenu()
{
	int num = 0;

	while (1)
	{
		cout << "Select menu" << endl;
		cout << "-----------------------" << endl;
		cout << "1) Create chat server" << endl;
		cout << "2) Join chat server" << endl;
		cout << "3) Exit program" << endl;
		cout << "-----------------------" << endl;
		cout << "Choose number => ";

		cin >> num;

		if ((num < 1) || (num > 3))
			cout << "Wrong menu number! Select menu again." << endl;

		Sleep(1000);

		system("cls");

		return num;
	}
}

void TCPChatServer()
{
	char recvBuff[BUFFERSIZE];
    char sendBuff[BUFFERSIZE];

	GetPortNum(Server, 1024, 49151);

	system("cls");
	Sleep(1000);

	WSADATA socketData;
	SOCKET serverSocket;
    SOCKET clientSocket;
	SOCKADDR_IN serverAddress;
	SOCKADDR_IN clientAddress;

	if (WSAStartup(0x202, &socketData) == SOCKET_ERROR)
	{
		cout << "Socket startup fail!" << endl;
		WSACleanup();
		exit(0);
	}

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddress.sin_port = htons(serverPort);

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (serverSocket == INVALID_SOCKET)
	{
		cout << "Cannot create socket!";
		closesocket(serverSocket);
		WSACleanup();
		exit(0);
	}

	if (bind(serverSocket, (struct sockaddr*) & serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Cannot Bind.");
		closesocket(serverSocket);
		WSACleanup();
		exit(0);
	}

	cout << "Server create success! Open server now..." << endl;

	while (1)
	{
		cout << "Server port number : " << serverPort << endl;

        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
			printf("Server listen fail!\n");
			closesocket(serverSocket);
			WSACleanup();
			exit(0);
		}

        cout << "Wait for connect client...";

		int clientAddressSize = sizeof(clientAddress);

        clientSocket = accept(serverSocket, (struct sockaddr*) & clientAddress, &clientAddressSize);

		if (clientSocket == INVALID_SOCKET)
        {
			cout << "Client accept fail! Return waiting...";
            Sleep(1000);
            continue;
        }

		cout << "Connect client!" << endl;
		cout << "Client Address:Port = " << inet_ntoa(clientAddress.sin_addr) << ":" << htons(clientAddress.sin_port) << endl;

        while (1)
        {
            recv(clientSocket, recvBuff, BUFFERSIZE, 0);

            if (!strcmp(recvBuff, "#exit"))
            {
                cout << "Client exit chat server. Return to waiting..." << endl;
                break;
            }

            cout << "Client send => " << recvBuff << endl;
            cout << "Server send => ";
			cin.getline(sendBuff, BUFFERSIZE);

            send(clientSocket, sendBuff, BUFFERSIZE, 0);

			if (!strcmp(sendBuff, "#exit"))
			{
				cout << "You exit chat server. Return to main menu..." << endl;
				Sleep(1000);
				closesocket(serverSocket);
				closesocket(clientSocket);
				WSACleanup();
				system("cls");
				return;
			}
        }

		Sleep(1000);

		closesocket(clientSocket);

		system("cls");
	}
}

void TCPChatClient()
{
	char recvBuff[BUFFERSIZE];
    char sendBuff[BUFFERSIZE];
	int connectPort = 0;

	system("cls");
	Sleep(1000);

	WSADATA socketData;
	SOCKET clientSocket;
	SOCKADDR_IN targetServerAddress;
	SOCKADDR_IN clientAddress;

	if (WSAStartup(0x202, &socketData) == SOCKET_ERROR)
	{
		cout << "Socket startup fail!" << endl;
		WSACleanup();
		exit(0);
	}

	cout << "Input server port number => ";
	cin >> connectPort;

	targetServerAddress.sin_family = AF_INET;
	targetServerAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	targetServerAddress.sin_port = htons(connectPort);

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (clientSocket == INVALID_SOCKET)
	{
		cout << "Cannot create socket!";
		closesocket(clientSocket);
		WSACleanup();
		exit(0);
	}

    if (connect(clientSocket, (struct sockaddr*) & targetServerAddress, sizeof(targetServerAddress)))
    {
        cout << "Cannot connet chat server!";
		closesocket(clientSocket);
		WSACleanup();
		return;
    }

	cout << "Server connect success!" << endl;
	cout << "Server port number : " << htons(targetServerAddress.sin_port) << endl;

    while (1)
    {
        cout << "You send => ";
        cin.getline(sendBuff, BUFFERSIZE);

        send(clientSocket, sendBuff, BUFFERSIZE, 0);

        if (!strcmp(sendBuff, "#exit"))
        {
            cout << "Exit chat server. Return to main menu..." << endl;
            break;
        }

        recv(clientSocket, recvBuff, BUFFERSIZE, 0);

		if (!strcmp(recvBuff, "#exit"))
		{
			cout << "Server Exit chat server. Return to main menu..." << endl;
			break;
		}

        cout << "Server send => " << recvBuff << endl;
    }

	Sleep(1000);

	closesocket(clientSocket);
	WSACleanup();

	system("cls");
}

int main(int argc, char* argv[])
{
	while (1)
	{
		switch (MainMenu())
		{
			case 1:
				TCPChatServer();
				break;
			case 2:
				TCPChatClient();
				break;
			case 3:
				exit(1);
				break;
			default:
				break;
		}
	}
}