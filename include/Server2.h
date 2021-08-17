#include <WS2tcpip.h>
#include <WinSock2.h>
#include <iostream>
#include <fstream>
#include <queue>
#include <filesystem>
#include <winsock.h>
#include <string>
#include <istream>
class Server2
{
public:
	Server2(std::string ipAddress, int port, std::string input_dir, std::string output_dir);

	~Server2();
	// Initialize winsock
	void init();
	

	// cleanup
	void cleanup();

	void receiveFiles();

	void sendFiles();
private:
	
	SOCKET CreateSocket(std::string m_ipAddress, int m_port);

	SOCKET WaitForConnection(SOCKET listening);

	SOCKET CreateClientSocket(std::string m_ipAddress, int m_port);

	SOCKET ConnectToPeer(SOCKET sock);

	std::string				m_ipAddress;
	int						m_port;
	std::string				output_dir;
	std::string				input_dir;
};