#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstring>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
#else
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #include <netdb.h>
  #define SOCKET int
  #define INVALID_SOCKET -1
  #define SOCKET_ERROR -1
  #define closesocket close
#endif

using namespace std;

string generate_plate() {
    string letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    string plate = "";
    for (int i = 0; i < 3; ++i) plate += letters[rand() % 26];
    plate += "-";
    for (int i = 0; i < 3; ++i) plate += to_string(rand() % 10);
    return plate;
}

int main(int argc, char* argv[]) {
    srand(time(0));
    string server_ip = "127.0.0.1";
    int port = 8080;

    if (argc > 1) server_ip = argv[1];
    if (argc > 2) port = atoi(argv[2]);

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed." << endl;
        return 1;
    }
#endif

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Socket creation error" << endl;
        return 1;
    }

    struct hostent *server_host = gethostbyname(server_ip.c_str());
    if (server_host == NULL) {
        cerr << "No such host: " << server_ip << endl;
        return 1;
    }

    sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr.s_addr, server_host->h_addr, server_host->h_length);

    while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        cout << "Connection Failed. Retrying in 2 seconds..." << endl;
        this_thread::sleep_for(chrono::seconds(2));
    }

    cout << "Connected to server " << server_ip << ":" << port << endl;
    
    vector<string> active_plates;

    while (true) {
        string plate;
        if (!active_plates.empty() && (rand() % 100 < 30)) {
            int idx = rand() % active_plates.size();
            plate = active_plates[idx];
            active_plates.erase(active_plates.begin() + idx);
            cout << "Sending plate (leaving): " << plate << endl;
        } else {
            plate = generate_plate();
            active_plates.push_back(plate);
            cout << "Sending plate (entering): " << plate << endl;
        }

        plate += "\n";
        send(sock, plate.c_str(), plate.length(), 0);

        int delay = 2 + (rand() % 4); // 2 to 5 seconds
        this_thread::sleep_for(chrono::seconds(delay));
    }

    closesocket(sock);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
