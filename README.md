# 🚗 Parking Lot System


## 📌 Características
- **Backend C++**: Gestiona la lógica del estacionamiento y conexiones simultáneas mediante multithreading.
- **Cliente C++**: Simulador de llegadas de vehículos (envío de patentes vía TCP Sockets).
- **Interfaz Streamlit**: UI en Python en tiempo real para visualizar ocupación, tiempos y facturación.
- **Dockerizado**: Construcción y ejecución simplificada a través de Docker y CMake.

## 🏗️ Arquitectura y Diagrama UML

El diseño se compone de tres módulos principales (Cliente, Servidor, UI). A continuación se presenta el Diagrama de Clases UML que ilustra las relaciones y estructuras de datos para cumplir con la rúbrica del proyecto.

> **Nota:** El archivo original del diagrama se encuentra en [`docs/diagrama_uml.md`](./docs/diagrama_uml.md)

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

## 🚀 Cómo ejecutar

### Usando Docker
El proyecto puede ser ejecutado y orquestado en contenedores mediante Docker Compose:
```bash
docker-compose up --build
```

### Compilación Manual
1. Construir el backend y el cliente usando CMake en el directorio `src/`.
2. Iniciar el servidor embebido al abrir la UI o ejecutando los binarios correspondientes.
3. Ejecutar la UI de Streamlit:
```bash
streamlit run src/ui/app.py
```
