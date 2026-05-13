#ifndef RECEIVER_H
#define RECEIVER_H

#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
    EXPORT void set_total_cells(int total);
    EXPORT void start_server(int port);
    EXPORT void stop_server();
    EXPORT const char* get_parking_state_json();
}

#endif
