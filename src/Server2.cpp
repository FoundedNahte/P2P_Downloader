#include "Server2.h"

Server2::Server2(std::string ipAddress, int port, std::string input, std::string output)
	: m_ipAddress(ipAddress), m_port(port), input_dir(input), output_dir(output)
{

}

Server2::~Server2()
{
	cleanup();
}

void Server2::init()
{
	WSADATA data;
	int wsInit = WSAStartup(MAKEWORD(2, 2), &data);
	if (wsInit != 0)
	{
		std::cerr << "error initializing winsock\n";
	}
	else
	{
		std::cout << "Successfully initialized winsock\n";
	}
}


SOCKET Server2::CreateSocket(std::string m_ipAddress, int m_port)
{
	SOCKET listening = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listening != INVALID_SOCKET)
	{
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(m_port);
		inet_pton(AF_INET, m_ipAddress.c_str(), &hint.sin_addr);

		int bindOk = bind(listening, (const sockaddr*)&hint, sizeof(hint));
		if (bindOk != SOCKET_ERROR)
		{
			int listenOk = listen(listening, SOMAXCONN);
			if (listenOk == SOCKET_ERROR)
			{
				return -1;
			}
		}
		else
		{
			return -1;
		}

	}
	std::cout << "Socket successfully created\n";
	return listening;
}

SOCKET Server2::CreateClientSocket(std::string m_ipAddress, int m_port)
{
	SOCKET listening = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listening != INVALID_SOCKET)
	{
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(m_port);
		inet_pton(AF_INET, m_ipAddress.c_str(), &hint.sin_addr);

		connect(listening, (const sockaddr*)&hint, sizeof(hint));
	}
	return listening;
}

SOCKET Server2::WaitForConnection(SOCKET listening)
{
	SOCKET client = accept(listening, NULL, NULL);
	std::cout << "accepted incoming connection";
	
	return client;
}

SOCKET Server2::ConnectToPeer(SOCKET sock)
{
	SOCKET peer = connect(sock, NULL, NULL);
	std::cout << "Connected to peer";
	return peer;
}

bool recvraw(SOCKET socket, char* buf, int buflen)
{
	while (buflen > 0)
	{
		int received = recv(socket, buf, buflen, 0);
		if (received < 1) return false;
		buf += received;
		buflen -= received;
	}
	return true;
}

bool sendraw(SOCKET socket, char* buf, int buflen)
{
	while (buflen > 0)
	{
		int sent = send(socket, buf, buflen, 0);
		if (sent < 1) return false;
		buf += sent;
		buflen -= sent;
	}
	return true;
}

bool helper(const std::filesystem::path& p, std::filesystem::file_status s)
{
	if (std::filesystem::is_regular_file(s))
		return true;
	else
		return false;
}
void Server2::sendFiles()
{
	std::string peer_output_dir;
	std::queue<std::string> in_directory_queue;
	std::queue<std::string> out_directory_queue;

	while(true)
	{
		SOCKET listening = CreateSocket(m_ipAddress, m_port);
		if (listening == INVALID_SOCKET)
		{
			break;
		}

		SOCKET peer = WaitForConnection(listening);
		if (peer != INVALID_SOCKET)
		{		
			uint32_t dataLength;
			recv(peer, (char*)&dataLength, sizeof(uint32_t), 0);
			dataLength = ntohl(dataLength);
			std::vector<char> buffer;
			buffer.resize(dataLength, 0x00);
			recv(peer, &(buffer[0]), dataLength, 0);
			
			peer_output_dir.assign(buffer.begin(), buffer.end());
			buffer.clear();

			uintmax_t totalSize = std::filesystem::directory_entry(input_dir).file_size();
			sendraw(peer, (char*)&totalSize, sizeof(int));
			uintmax_t totalBytesSent = 0;
			in_directory_queue.push(input_dir);
			out_directory_queue.push(peer_output_dir);


			while (!in_directory_queue.empty())
			{
				in_directory_queue.pop();
				out_directory_queue.pop();
				try
				{
					for (auto& it : std::filesystem::directory_iterator(input_dir))
					{
						std::string output_file = peer_output_dir + it.path().string().substr(input_dir.length(), it.path().string().length() - 1);
						dataLength = htonl(output_file.length());
						send(peer, (char*)&dataLength, sizeof(uint32_t), 0);
						send(peer, output_file.c_str(), output_file.size(), 0);


						int isFile = helper(it, symlink_status(it));
						send(peer, (char*)&isFile, sizeof(int), 0);

						if (helper(it, symlink_status(it)))
						{
							std::ifstream source(it.path().string(), std::ios::binary);
							source.seekg(0, std::ios::end);
							dataLength = htonl(int(source.tellg()));
							source.seekg(0);
							send(peer, (char*)&dataLength, sizeof(uint32_t), 0);
							if (ntohl(dataLength) > 0)
							{							
								std::cout << "[SENDING FILE] " << it.path().string() << " " << "[FILESIZE] " << dataLength << "\n";

								char* buffer = new char[ntohl(dataLength)];
								source.read(buffer, ntohl(dataLength));
								sendraw(peer, buffer, ntohl(dataLength));
								delete[] buffer;
								source.close();
							}
							else
							{
								std::cout << "[SENDING FILE] " << it.path().string() << " " << "[FILESIZE] " << dataLength << "\n";
							}
						}
						else
						{
							std::cout << std::endl << "SENDING DIRECTORY: " << it.path().string() << "\n";
							std::string temp = peer_output_dir + it.path().string().substr(input_dir.length(), it.path().string().length() - 1);
							std::cout << "PEER DIRECTORY: " << temp << std::endl;
							in_directory_queue.push(input_dir + it.path().string().substr(input_dir.length(), it.path().string().length() - 1));
							std::cout << it.path().string().substr(input_dir.length(), it.path().string().length() - 1) << std::endl;
							std::cout << input_dir + it.path().string().substr(input_dir.length(), it.path().string().length() - 1) << std::endl;
							out_directory_queue.push(temp);
						}
						int ready = 1;
						send(peer, (char*)&ready, sizeof(int), 0);
					}
					if (!in_directory_queue.empty())
					{
						peer_output_dir = out_directory_queue.front();
						input_dir = in_directory_queue.front();
						std::cout << std::endl << output_dir << " " << input_dir;
					}
				}
				catch(const std::exception &exc)
				{
					std::cerr << exc.what();
					break;
				}
			}
		}

	}

}



void Server2::receiveFiles()
{
	std::string peer_ipAddress;
	std::cout << "Enter IP address of peer\n";
	std::cin >> peer_ipAddress;
	int peer_port;
	std::cout << "Enter port number of peer\n";
	std::cin >> peer_port;
	while (true)
	{
		SOCKET peer = CreateClientSocket(peer_ipAddress, peer_port);

		if (peer == INVALID_SOCKET)
		{
			break;
		}

		else
		{
			uint32_t dataLength = htonl(output_dir.size());

			send(peer, (char*)&dataLength, sizeof(uint32_t), 0);
			send(peer, output_dir.c_str(), output_dir.size(), 0);
			
			uintmax_t totalSize;
			recv(peer, (char*)&totalSize, sizeof(int), 0);
			int bytesReceived = 0;
			uintmax_t totalBytesReceived = 0;
			std::vector<char> buffer;
			std::string name;
			int isFile;
			int ready;
			while (totalBytesReceived < totalSize)
			{
				recv(peer, (char*)&dataLength, sizeof(uint32_t), 0);
				dataLength = ntohl(dataLength);
				std::cout << "[NAME BUFFER SIZE] " << dataLength << "\n";
				buffer.resize(dataLength, 0x00);

				recv(peer, &(buffer[0]), dataLength, 0);
				name.assign(buffer.begin(), buffer.end());
				recv(peer, (char*)&isFile, sizeof(int), 0);
				buffer.clear();
				if (isFile)
				{
					try
					{

						recv(peer, (char*)&dataLength, sizeof(uint32_t), 0);
						dataLength = ntohl(dataLength);
						std::cout << "[DATA BUFFER SIZE] " << dataLength << "\n";
						std::cout << name << "\n";

						if (dataLength > 0)
						{
							char* buffer = new char[dataLength];
							recvraw(peer, buffer, dataLength);
							totalBytesReceived += dataLength;
							std::ofstream dest(name, std::ios::binary);
							dest.write(buffer, dataLength);
							delete[] buffer;
							dest.close();
						}
						else
						{
							std::ofstream dest(name, std::ios::binary);
							dest.close();
						}
					}
					catch (const std::bad_alloc)
					{
						break;
					}
				}
				else
				{
					std::cout << "Creating Directory: " << name << "\n";
					std::filesystem::create_directory(name);

				}

				recv(peer, (char*)&ready, sizeof(int), 0);
			}
		}
	}
	std::cout << "[FINISHED]" << "\n";
}

void Server2::cleanup()
{
	WSACleanup();
}
