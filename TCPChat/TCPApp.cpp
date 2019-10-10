// Set common buffer size
#define BUFFERSIZE 1024

#include <cstdlib>
#include <iostream>
#include <WinSock2.h>

// Disable warning code 4996
#pragma comment (lib, "ws2_32.lib")
#pragma warning(disable: 4996)

using namespace std;

// Define enum for mode
typedef enum Type { Server, Client }Type;

unsigned short serverPort = 0;
unsigned short clientPort = 0;

// Get port number to set port number value
// Server port value : 1024 ~ 49151
// Client port value : 49152 ~ 65535
// Save value global variable (serverPort, clientPort)
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

// Create and select menu interface
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

// Working server mode
void TCPChatServer()
{
	char recvBuff[BUFFERSIZE]; // Receive Buffer
    char sendBuff[BUFFERSIZE]; // Send Buffer

	GetPortNum(Server, 1024, 49151);

	system("cls");
	Sleep(1000);

	WSADATA socketData;
	SOCKET serverSocket;
    SOCKET clientSocket;
	SOCKADDR_IN serverAddress;
	SOCKADDR_IN clientAddress;

	// Initialize Win Socket library
	if (WSAStartup(0x202, &socketData) == SOCKET_ERROR)
	{
		cout << "Socket startup fail!" << endl;
		WSACleanup();
		exit(0);
	}

	// Set server protocol info. IP Address fix 127.0.0.1
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddress.sin_port = htons(serverPort);

	// Create server socket (TCP)
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	// Check validity of server socket
	if (serverSocket == INVALID_SOCKET)
	{
		cout << "Cannot create socket!";
		closesocket(serverSocket);
		WSACleanup();
		exit(0);
	}

	// Bind server socket
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

		// Listen server
        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
			printf("Server listen fail!\n");
			closesocket(serverSocket);
			WSACleanup();
			exit(0);
		}

        cout << "Wait for connect client...";

		int clientAddressSize = sizeof(clientAddress);

		// Accept client when client send request
        clientSocket = accept(serverSocket, (struct sockaddr*) & clientAddress, &clientAddressSize);

		// Check validity of client socket
		if (clientSocket == INVALID_SOCKET)
        {
			cout << "Client accept fail! Return waiting...";
            Sleep(1000);
            continue;
        }

		cout << "Connect client!" << endl;
		cout << "Client Address:Port = " << inet_ntoa(clientAddress.sin_addr) << ":" << htons(clientAddress.sin_port) << endl << endl;
		cin.ignore(); // Clear console input buffer

        while (1)
        {
            recv(clientSocket, recvBuff, BUFFERSIZE, 0); // Receive client message

			// If receive "#exit", close chat instance & return to wait. Chat server is not close.
            if (!strcmp(recvBuff, "#exit"))
            {
                cout << "Client exit chat server. Return to waiting..." << endl;
                break;
            }

            cout << "Client send => " << recvBuff << endl;
            cout << "Server send => ";
			cin.getline(sendBuff, BUFFERSIZE); // Get console input message

            send(clientSocket, sendBuff, BUFFERSIZE, 0); // Send message to client

			// If server input "#exit", close chat instance & close chat server. Return to main menu.
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

// Working client mode
void TCPChatClient()
{
	char recvBuff[BUFFERSIZE]; // Recieve buffer
    char sendBuff[BUFFERSIZE]; // Send buffer
	int connectPort = 0; // Store server port number

	system("cls");
	Sleep(1000);

	WSADATA socketData;
	SOCKET clientSocket;
	SOCKADDR_IN targetServerAddress;
	SOCKADDR_IN clientAddress;

	// Initialize Win Socket library
	if (WSAStartup(0x202, &socketData) == SOCKET_ERROR)
	{
		cout << "Socket startup fail!" << endl;
		WSACleanup();
		exit(0);
	}

	// Get server port to connect
	cout << "Input server port number => ";
	cin >> connectPort;

	// Set target server protocol info. IP Address fix 127.0.0.1
	targetServerAddress.sin_family = AF_INET;
	targetServerAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	targetServerAddress.sin_port = htons(connectPort);

	// Create client socket (TCP)
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	// Check validity of client socket
	if (clientSocket == INVALID_SOCKET)
	{
		cout << "Cannot create socket!";
		closesocket(clientSocket);
		WSACleanup();
		exit(0);
	}

	// Connect target server
    if (connect(clientSocket, (struct sockaddr*) & targetServerAddress, sizeof(targetServerAddress)))
    {
        cout << "Cannot connet chat server!";
		Sleep(1000);
		closesocket(clientSocket);
		WSACleanup();
		system("cls");
		return;
    }

	cout << "Server connect success!" << endl;
	cout << "Server port number : " << htons(targetServerAddress.sin_port) << endl << endl;
	cin.ignore(); // Clear console input buffer

    while (1)
    {
        cout << "You send => ";
        cin.getline(sendBuff, BUFFERSIZE); // Get console input message

        send(clientSocket, sendBuff, BUFFERSIZE, 0); // Send message to client

		// If send "#exit", exit chat server & return to main menu.
        if (!strcmp(sendBuff, "#exit"))
        {
            cout << "Exit chat server. Return to main menu..." << endl;
            break;
        }

        recv(clientSocket, recvBuff, BUFFERSIZE, 0); // Receive client message

		// If receive "#exit", close chat instance & return to wait. Chat server is not close.
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