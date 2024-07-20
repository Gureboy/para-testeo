import os
import json
import requests
from flask import Flask, request, jsonify, render_template
from flask_socketio import SocketIO, emit

app = Flask(__name__)
socketio = SocketIO(app)

# Ruta donde se guardarán los archivos de datos
DATA_FOLDER = 'datos'

# Asegurarse de que la carpeta de datos exista
if not os.path.exists(DATA_FOLDER):
    os.makedirs(DATA_FOLDER)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/guardar_datos', methods=['POST'])
def guardar_datos():
    try:
        datos = request.json
        if not datos or 'id' not in datos:
            return jsonify({'error': 'Datos inválidos'}), 400

        nombre_archivo = f"{datos['id']}.json"
        ruta_archivo = os.path.join(DATA_FOLDER, nombre_archivo)
        
        with open(ruta_archivo, 'w') as archivo:
            json.dump(datos, archivo)
        
        # Emitir evento para actualizar la UI
        socketio.emit('nuevos_datos', {'mensaje': 'Datos guardados', 'datos': datos})
        
        return jsonify({'message': 'Datos guardados'}), 200
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/enviar_datos', methods=['GET'])
def enviar_datos():
    servidor_principal = 'http://servidor-principal.com/recibir_datos'
    archivos = os.listdir(DATA_FOLDER)
    errores = []

    for archivo in archivos:
        ruta_archivo = os.path.join(DATA_FOLDER, archivo)
        try:
            with open(ruta_archivo, 'r') as f:
                datos = json.load(f)
                response = requests.post(servidor_principal, json=datos)
                response.raise_for_status()
            # Eliminar archivo después de enviarlo exitosamente
            os.remove(ruta_archivo)
        except Exception as e:
            errores.append({'archivo': archivo, 'error': str(e)})

    if errores:
        return jsonify({'message': 'Datos enviados con errores', 'errores': errores}), 207
    else:
        return jsonify({'message': 'Datos enviados exitosamente'}), 200

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000)
