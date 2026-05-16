# Diagrama UML de Arquitectura

El siguiente diagrama de clases muestra la arquitectura del sistema de estacionamiento, detallando la interacción entre el cliente C++, la librería del servidor C++ y la interfaz de Python (Streamlit).

```mermaid
classDiagram
    class ClientApp {
        <<C++ Client>>
        +main()
        +send_car_arrival(plate: string)
    }

    class ServerLib {
        <<C++ Backend Library>>
        -parking_map: map~string, CarInfo~
        -available_cells: vector~bool~
        -server_running: atomic~bool~
        -parking_mutex: mutex
        +start_server(port: int)
        +stop_server()
        +set_total_cells(total: int)
        +get_parking_state_json() : string
        -server_loop()
        -handle_client(socket)
    }

    class StreamlitUI {
        <<Python UI>>
        +load_library()
        +update_dashboard()
        +render_visuals()
    }

    class CarInfo {
        <<struct>>
        +cell_id: int
        +entry_time: long long
    }

    ClientApp --> ServerLib : TCP Sockets (Envía patentes)
    StreamlitUI --> ServerLib : C-API Calls (ctypes)
    ServerLib *-- CarInfo : Gestiona estado
```
