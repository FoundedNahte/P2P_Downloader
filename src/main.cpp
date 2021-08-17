#include <iostream>
#include <string>
#include <filesystem>
#include "Server2.h"

using namespace std;


int main()
{
	string m_ipAddress;
	int m_port;
	string output_dir;
	string input_dir;
	int input;
	cout << "Enter your public IP address\n";
	cin >> m_ipAddress;
	cout << "Enter a port number\n";
	cin >> m_port;
	cout << "Enter directory for downloads:\n";
	cin >> output_dir;
	cout << "Enter directory for hosting (Enter \"NULL\" without the quotes if not hosting a directory):\n";
	cin >> input_dir;
	cout << "Enter 1 for hosting files, or 0 for receiving files\n";
	cin >> input;
	/*
	ifstream inFile;
	inFile.open("C:/Users/Ethan/Desktop/repo/P2P_Downloader/src/config.txt");
	
	if (!inFile)
	{
		cerr << "Unable to open config txt file\n";
		exit(1);
	}
	else
	{
		inFile >> m_ipAddress;
		inFile >> m_port;
		inFile >> output_dir;
		inFile >> input_dir;
		inFile >> input;
	}

	*/
	Server2 server(m_ipAddress, m_port, input_dir, output_dir);
	if (input)
	{
		server.init();
		server.sendFiles();
	}
	else
	{
		server.init();
		server.receiveFiles();
	}
	system("pause");
	return 0;
}

