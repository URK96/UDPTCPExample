
#define FILEPATHMAXLENGTH 255
#define BUFFERSIZE 100

#include <cstdlib>
#include <iostream>
#include <windows.h>
#include <WinSock2.h>

#pragma comment (lib, "ws2_32.lib")

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

	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_UDP);

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
			socket.send("ERROR", 6);
			socket.disconnect();
			continue;
		}

		fseek(file, 0, SEEK_END);
		fileSize = ftell(file);
		fseek(file, 0, SEEK_SET);

		_snprintf(buff, sizeof(buff), "%d", fileSize);
		socket.sendTo(buff, sizeof(char) * BUFFERSIZE, sourceAddr, sourcePort);

		cout << "Start file tranfer" << endl;
		socket.send("START", 6);

		while ((readBytes = fread(buff, sizeof(char), BUFFERSIZE, file)) > 0)
		{
			socket.send(buff, readBytes);
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
	char checkConnect[CHECKSIZE];
	char requestFile[FILEPATHMAXLENGTH];
	char desFile[FILEPATHMAXLENGTH];
	string serverAddr;
	unsigned short serverPort;
	char buff[BUFFERSIZE];

	system("cls");
	Sleep(1000);

	try
	{
		UDPSocket socket;

		cout << "Input server address => ";
		cin >> serverAddr;
		cout << "Input server port number (1024 ~ 49151) => ";
		cin >> serverPort;

		cout << "Wait for connect server...";

		socket.sendTo(checkConnect, CHECKSIZE, serverAddr, serverPort);

		cout << "Connect server!" << endl;
		cout << "Server Address:Port = " << serverAddr << ":" << serverPort << endl;

		cout << "Input want to receive file path => ";
		cin >> requestFile;
		cout << "Input want to save file path => ";
		cin >> desFile;

		socket.sendTo(requestFile, FILEPATHMAXLENGTH, serverAddr, serverPort);

		FILE* file;
		int readBytes;
		int totalRecvBytes = 0;
		int fileTotalBytes = 0;

		if ((file = fopen(desFile, "wb")) == NULL)
		{
			cout << "Cannot create file!" << endl;
			return;
		}

		cout << "Start file tranfer" << endl;

		socket.recvFrom(buff, sizeof(char) * BUFFERSIZE, serverAddr, serverPort);
		fileTotalBytes = atoi(buff);

		while (totalRecvBytes < fileTotalBytes)
		{
			readBytes = socket.recvFrom(buff, sizeof (char) * BUFFERSIZE, serverAddr, serverPort);

			fwrite(buff, sizeof(char), readBytes, file);
			totalRecvBytes += readBytes;
			cout << "File transfer progress : " << totalRecvBytes << "B" << endl;
		}

		fflush(file);
		fclose(file);

		cout << "File transfer complete!" << endl;

		Sleep(1000);

		system("cls");
	}
	catch (SocketException& e)
	{
		cerr << e.what() << endl;
	}
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