import os
import json
import requests
from flask import Flask, request

app = Flask(__name__)

# Ruta donde se guardarán los archivos de datos
DATA_FOLDER = 'datos'

# Asegurarse de que la carpeta de datos exista
if not os.path.exists(DATA_FOLDER):
    os.makedirs(DATA_FOLDER)

@app.route('/guardar_datos', methods=['POST'])
def guardar_datos():
    # Recibe datos del ESP8266
    datos = request.json
    # Genera un nombre de archivo único
    nombre_archivo = f"{datos['id']}.json"
    ruta_archivo = os.path.join(DATA_FOLDER, nombre_archivo)
    
    # Guarda los datos en un archivo JSON
    with open(ruta_archivo, 'w') as archivo:
        json.dump(datos, archivo)
    
    return 'Datos guardados', 200

@app.route('/enviar_datos', methods=['GET'])
def enviar_datos():
    # Envía todos los archivos de datos al servidor principal
    servidor_principal = 'http://servidor-principal.com/recibir_datos'
    archivos = os.listdir(DATA_FOLDER)
    
    for archivo in archivos:
        ruta_archivo = os.path.join(DATA_FOLDER, archivo)
        with open(ruta_archivo, 'r') as f:
            datos = json.load(f)
            # Envía los datos al servidor principal
            requests.post(servidor_principal, json=datos)
    
    return 'Datos enviados', 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
