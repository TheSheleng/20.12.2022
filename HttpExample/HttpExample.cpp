#pragma comment (lib, "Ws2_32.lib")
#include <Winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

const string FindPrint(const string&, const string&);

int main()
{
    setlocale(0, "ru");
    WSADATA wsaData;
    int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (err != 0) 
    {

        cout << "WSAStartup failed with error: " << err << endl;
        return 1;
    }  

    const string ParsTo[] =
    {
        "\"id\"",
        "\"name\"",
        "\"country\"",
        "\"lon\"",
        "\"lat\"",
        "\"temp_min\"",
        "\"temp_max\"",
        "\"sunset\"",
        "\"sunrise\"",
    };

    while (true)
    {
        char hostname[256] = "api.openweathermap.org";
        addrinfo* result = NULL;

        addrinfo hints;
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        int iResult = getaddrinfo(hostname, "http", &hints, &result);
        if (iResult != 0) {
            cout << "getaddrinfo failed with error: " << iResult << endl;
            WSACleanup();
            return 3;
        }

        SOCKET connectSocket = INVALID_SOCKET;
        addrinfo* ptr = NULL;

        for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
        {
            connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
            if (connectSocket == INVALID_SOCKET) {
                printf("socket failed with error: %ld\n", WSAGetLastError());
                WSACleanup();
                return 1;
            }

            iResult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
            if (iResult == SOCKET_ERROR) {
                closesocket(connectSocket);
                connectSocket = INVALID_SOCKET;
                continue;
            }
            break;
        }

        string city;
        cout << "Enter city name (enter \'end\' to end): ";
        cin >> city;

        if (city == "end") break;

        string uri = "/data/2.5/weather?q=" + city + "&appid=75f6e64d49db78658d09cb5ab201e483&mode=JSON&units=metric";
        string request = "GET " + uri + " HTTP/1.1\n";
        request += "Host: " + string(hostname) + "\n";
        request += "Accept: */*\n";
        request += "Accept-Encoding: gzip, deflate, br\n";
        request += "Connection: close\n";
        request += "\n";

        if (send(connectSocket, request.c_str(), request.length(), 0) == SOCKET_ERROR) {
            cout << "send failed: " << WSAGetLastError() << endl;
            closesocket(connectSocket);
            WSACleanup();
            return 5;
        }

        string response;

        const size_t BUFFERSIZE = 1024;
        char resBuf[BUFFERSIZE];

        int respLength;

        do {
            respLength = recv(connectSocket, resBuf, BUFFERSIZE, 0);
            if (respLength > 0) {
                response += string(resBuf).substr(0, respLength);
            }
            else {
                cout << "recv failed: " << WSAGetLastError() << endl;
                closesocket(connectSocket);
                WSACleanup();
                return 6;
            }

        } while (respLength == BUFFERSIZE);

        fstream file("log.txt", ios::in | ios::app);

        if (file.is_open())
        {
            for (const string& s : ParsTo)
            {
                const string res = FindPrint(response, s);

                cout << res;
                file << res;
            }

            cout << endl;
            file << endl;
            file.close();
        }
        else
        {
            cout << "open file failed: " << WSAGetLastError() << endl;
            closesocket(connectSocket);
            WSACleanup();
            return 8;
        }

        //отключает отправку и получение сообщений сокетом
        iResult = shutdown(connectSocket, SD_BOTH);
        if (iResult == SOCKET_ERROR) {
            cout << "shutdown failed: " << WSAGetLastError() << endl;
            closesocket(connectSocket);
            WSACleanup();
            return 7;
        }
        closesocket(connectSocket);
    }

    WSACleanup();
}

const string FindPrint(const string& sours, const string& find)
{
    string str;

    str = find + ": ";
    for (auto i = sours.begin() + sours.find(find) + find.length() + 1; *i != ',' && *i != '}'; ++i) str += *i;
    str += '\n';

    return str;
}