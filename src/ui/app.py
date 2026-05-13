import streamlit as st
import ctypes
import json
import time
import os
import platform

st.set_page_config(page_title="Sistema de Parqueadero", layout="wide")

# Determine library path based on OS
lib_name = "libparking_server_lib.so"
if platform.system() == "Windows":
    lib_name = "parking_server_lib.dll"

# Paths to try
paths_to_try = [
    os.path.join(os.getcwd(), "build", "Release", lib_name),
    os.path.join(os.getcwd(), "build", lib_name),
    os.path.join(os.getcwd(), lib_name),
    os.path.join("/app/build", lib_name)
]

@st.cache_resource
def load_lib():
    for p in paths_to_try:
        if os.path.exists(p):
            lib = ctypes.CDLL(p)
            lib.get_parking_state_json.restype = ctypes.c_char_p
            return lib
    return None

lib = load_lib()

RATE_PER_SECOND = 1

st.title("🚗 Sistema de Parqueadero")

st.sidebar.header("Configuración")
max_cells = st.sidebar.number_input("Número de Celdas", min_value=1, max_value=100, value=20)
port = st.sidebar.number_input("Puerto del Servidor TCP", min_value=1024, max_value=65535, value=8080)

if not lib:
    st.error(f"No se encontró la librería C++ ({lib_name}). Asegúrate de compilar el proyecto primero.")
    st.stop()

if 'server_started' not in st.session_state:
    lib.set_total_cells(max_cells)
    lib.start_server(port)
    st.session_state['server_started'] = True

# Read state from C++
state_json = lib.get_parking_state_json()
if state_json:
    try:
        state = json.loads(state_json.decode('utf-8'))
    except:
        state = []
else:
    state = []

# Map occupied cells for easy lookup
occupied = {car["cell"]: car for car in state}

cols_per_row = 5

st.markdown("""
    <style>
    .cell {
        border-radius: 10px;
        padding: 15px;
        margin: 10px 0;
        text-align: center;
        box-shadow: 2px 2px 5px rgba(0,0,0,0.1);
    }
    .free {
        background-color: #d4edda;
        color: #155724;
        border: 2px solid #c3e6cb;
    }
    .occupied {
        background-color: #f8d7da;
        color: #721c24;
        border: 2px solid #f5c6cb;
    }
    .cell h3 { margin: 0 0 5px 0; font-size: 1.2rem; }
    .cell p { margin: 0; font-size: 0.9rem; }
    </style>
""", unsafe_allow_html=True)

current_time = int(time.time())

st.write(f"### Estado actual ({len(occupied)}/{max_cells} ocupadas)")

for i in range(0, max_cells, cols_per_row):
    cols = st.columns(cols_per_row)
    for j in range(cols_per_row):
        cell_idx = i + j
        if cell_idx >= max_cells:
            break
            
        with cols[j]:
            if cell_idx in occupied:
                car = occupied[cell_idx]
                elapsed = current_time - car['entry_time']
                value = elapsed * RATE_PER_SECOND
                st.markdown(f'''
                    <div class="cell occupied">
                        <h3>Celda {cell_idx}</h3>
                        <p><strong>{car["plate"]}</strong></p>
                        <p>⏱️ {elapsed}s</p>
                        <p>💵 ${value}</p>
                    </div>
                ''', unsafe_allow_html=True)
            else:
                st.markdown(f'''
                    <div class="cell free">
                        <h3>Celda {cell_idx}</h3>
                        <p>LIBRE</p>
                        <br><br>
                    </div>
                ''', unsafe_allow_html=True)

# Auto refresh to keep timer updated
time.sleep(1)
st.rerun()
