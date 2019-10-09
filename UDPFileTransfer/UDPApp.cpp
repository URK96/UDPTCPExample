
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
		cout << "1) Start server" << endl;
		cout << "2) Start client" << endl;
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

void UDPFileServer()
{
	char requestFile[FILEPATHMAXLENGTH];
	char buff[BUFFERSIZE];

	GetPortNum(Server, 1024, 49151);

	system("cls");
	Sleep(1000);

	WSADATA socketData;
	SOCKET serverSocket;
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

	serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

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
	cout << "Server port number : " << serverPort << endl;

	while (1)
	{
		cout << "Wait for connect client...";

		int clientAddressSize = sizeof(clientAddress);

		while (1)
		{
			recvfrom(serverSocket, buff, BUFFERSIZE, 0, (struct sockaddr*) & clientAddress, &clientAddressSize);
			
			if (strcmp(buff, "Connect") == 0)
				break;
		}

		cout << "Connect client!" << endl;
		cout << "Client Address:Port = " << inet_ntoa(clientAddress.sin_addr) << ":" << htons(clientAddress.sin_port) << endl;
		cout << "Wait for client file request..." << endl;

		recvfrom(serverSocket, requestFile, FILEPATHMAXLENGTH, 0, (struct sockaddr*) & clientAddress, &clientAddressSize);

		cout << "Request File : " << requestFile << endl;

		FILE* file;
		int fileSize = 0;
		int readBytes;
		int totalSendBytes = 0;

		if ((file = fopen(requestFile, "rb")) == NULL)
		{
			cout << "Cannot find file!";
			continue;
		}

		fseek(file, 0, SEEK_END);
		fileSize = ftell(file);
		fseek(file, 0, SEEK_SET);

		_snprintf(buff, sizeof(buff), "%d", fileSize);
		sendto(serverSocket, buff, BUFFERSIZE, 0, (struct sockaddr*) & clientAddress, sizeof(clientAddress));

		cout << "Start file tranfer" << endl;

		while ((readBytes = fread(buff, sizeof(char), BUFFERSIZE, file)) > 0)
		{
			sendto(serverSocket, buff, readBytes, 0, (struct sockaddr*) & clientAddress, sizeof(clientAddress));
			totalSendBytes += readBytes;
			cout << "File transfer progress : " << totalSendBytes << "B" << endl;
		}

		cout << "File transfer complete!" << endl;

		Sleep(1000);

		system("cls");
	}
}

void UDPFileClient()
{
	char requestFile[FILEPATHMAXLENGTH];
	char saveFile[FILEPATHMAXLENGTH];
	char buff[BUFFERSIZE];
	int connectPort = 0;

	system("cls");
	Sleep(1000);

	WSADATA socketData;
	SOCKET clientSocket;
	SOCKADDR_IN serverAddress;
	SOCKADDR_IN clientAddress;

	if (WSAStartup(0x202, &socketData) == SOCKET_ERROR)
	{
		cout << "Socket startup fail!" << endl;
		WSACleanup();
		exit(0);
	}

	cout << "Input server port number => ";
	cin >> connectPort;

	clientAddress.sin_family = AF_INET;
	clientAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	clientAddress.sin_port = htons(connectPort);

	clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (clientSocket == INVALID_SOCKET)
	{
		cout << "Cannot create socket!";
		closesocket(clientSocket);
		WSACleanup();
		exit(0);
	}

	cout << "Server connect success!" << endl;
	cout << "Server port number : " << serverPort << endl;

	int serverAddressSize = sizeof(serverAddress);

	sendto(clientSocket, "Connect", 8, 0, (struct sockaddr*) & clientAddress, sizeof(clientAddress));

	cout << "Input want to receive file => ";
	cin >> requestFile;

	cout << "Input want to save file => ";
	cin >> saveFile;

	cout << "Request File : " << requestFile << endl;
	cout << "Save File : " << saveFile << endl;

	sendto(clientSocket, requestFile, FILEPATHMAXLENGTH, 0, (struct sockaddr*) & clientAddress, sizeof(clientAddress));

	FILE* file;
	int fileSize = 0;
	int readBytes;
	int bufferSize = 0;
	int totalRecvBytes = 0;

	if ((file = fopen(saveFile, "wb")) == NULL)
	{
		cout << "Cannot create file!";
	}

	recvfrom(clientSocket, buff, BUFFERSIZE, 0, (struct sockaddr*) & serverAddress, &serverAddressSize);

	fileSize = atol(buff);

	cout << "Start file tranfer" << endl;

	while (totalRecvBytes < fileSize)
	{
		if ((fileSize - totalRecvBytes) > BUFFERSIZE)
			bufferSize = BUFFERSIZE;
		else
			bufferSize = fileSize - totalRecvBytes;

		readBytes = recvfrom(clientSocket, buff, bufferSize, 0, (struct sockaddr*) & serverAddress, &serverAddressSize);
		fwrite(buff, sizeof(char), readBytes, file);
		totalRecvBytes += readBytes;
		cout << "File transfer progress : " << totalRecvBytes << "B" << endl;
	}

	cout << "File transfer complete!" << endl;

	fflush(file);
	fclose(file);

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
				UDPFileServer();
				break;
			case 2:
				UDPFileClient();
				break;
			case 3:
				exit(1);
				break;
			default:
				break;
		}
	}
}