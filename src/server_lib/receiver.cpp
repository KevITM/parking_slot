#include "receiver.h"
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <sstream>
#include <cstring>
#include <atomic>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <unistd.h>
  #define SOCKET int
  #define INVALID_SOCKET -1
  #define SOCKET_ERROR -1
  #define closesocket close
#endif

using namespace std;

struct CarInfo {
    int cell_id;
    long long entry_time;
};

// Global state
map<string, CarInfo> parking_map; // plate -> CarInfo
vector<bool> available_cells;
int max_cells = 20;
mutex parking_mutex;
atomic<bool> server_running(false);
thread server_thread;
string current_json_state = "[]";
SOCKET server_fd = INVALID_SOCKET;

// Forward declaration
void handle_client(SOCKET client_socket);

void set_total_cells(int total) {
    lock_guard<mutex> lock(parking_mutex);
    max_cells = total;
    available_cells.assign(max_cells, true);
    parking_map.clear();
    current_json_state = "[]";
}

void update_json_state() {
    // Call this inside a lock
    stringstream ss;
    ss << "[";
    bool first = true;
    for (auto const& [plate, info] : parking_map) {
        if (!first) ss << ",";
        ss << "{\"plate\":\"" << plate << "\",\"cell\":" << info.cell_id 
           << ",\"entry_time\":" << info.entry_time << "}";
        first = false;
    }
    ss << "]";
    current_json_state = ss.str();
}

void server_loop(int port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed." << endl;
        return;
    }
#endif

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        cerr << "Error creating socket." << endl;
        return;
    }

    int opt = 1;
#ifdef _WIN32
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        cerr << "Bind failed." << endl;
        return;
    }

    if (listen(server_fd, 5) == SOCKET_ERROR) {
        cerr << "Listen failed." << endl;
        return;
    }

    server_running = true;
    while (server_running) {
        sockaddr_in client_addr;
#ifdef _WIN32
        int addrlen = sizeof(client_addr);
#else
        socklen_t addrlen = sizeof(client_addr);
#endif
        SOCKET client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
        if (client_socket == INVALID_SOCKET) {
            if (server_running) {
                cerr << "Accept failed." << endl;
            }
            continue;
        }

        thread(handle_client, client_socket).detach();
    }

#ifdef _WIN32
    WSACleanup();
#endif
}

void handle_client(SOCKET client_socket) {
    char buffer[1024] = {0};
    while (server_running) {
        memset(buffer, 0, sizeof(buffer));
        int valread = recv(client_socket, buffer, 1024, 0);
        if (valread <= 0) {
            break; // Client disconnected or error
        }

        string plate(buffer);
        // Trim newline if present
        plate.erase(plate.find_last_not_of("\n\r ") + 1);

        if (plate.empty()) continue;

        auto now = chrono::system_clock::now();
        long long timestamp = chrono::duration_cast<chrono::seconds>(now.time_since_epoch()).count();

        lock_guard<mutex> lock(parking_mutex);
        
        // Find if plate exists
        if (parking_map.find(plate) != parking_map.end()) {
            // Car leaves
            int cell = parking_map[plate].cell_id;
            available_cells[cell] = true;
            parking_map.erase(plate);
            cout << "[Server] Plate " << plate << " left cell " << cell << endl;
        } else {
            // Car enters, find free cell
            int free_cell = -1;
            for (int i = 0; i < max_cells; ++i) {
                if (available_cells[i]) {
                    free_cell = i;
                    break;
                }
            }
            
            if (free_cell != -1) {
                available_cells[free_cell] = false;
                parking_map[plate] = {free_cell, timestamp};
                cout << "[Server] Plate " << plate << " entered cell " << free_cell << endl;
            } else {
                cout << "[Server] Parking Full! Plate " << plate << " rejected." << endl;
            }
        }
        update_json_state();
    }
    closesocket(client_socket);
}

void start_server(int port) {
    if (!server_running) {
        if (available_cells.empty()) {
            set_total_cells(max_cells);
        }
        server_thread = thread(server_loop, port);
    }
}

void stop_server() {
    server_running = false;
    if (server_fd != INVALID_SOCKET) {
        closesocket(server_fd);
    }
    if (server_thread.joinable()) {
        server_thread.join();
    }
}

const char* get_parking_state_json() {
    lock_guard<mutex> lock(parking_mutex);
    static string returned_json;
    returned_json = current_json_state;
    return returned_json.c_str();
}
