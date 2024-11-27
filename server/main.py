import os
import time
import datetime
import json
import win32pipe
import win32file
import threading

import win32security

from settings import CACHE_LIFE_TIME, PIPE_NAME


class FileMonitorServer:
    def __init__(self):
        self.cache = dict()
        self.cache_life_stamp = dict()
        self.lock = threading.Lock()

    def create_pipe(self):
        # Дозволи для локальної мережі
        security_attributes = win32security.SECURITY_ATTRIBUTES()
        security_descriptor = win32security.SECURITY_DESCRIPTOR()
        security_descriptor.Initialize()
        security_descriptor.SetSecurityDescriptorDacl(1, None, 0)
        security_attributes.SECURITY_DESCRIPTOR = security_descriptor

        return win32pipe.CreateNamedPipe(
            PIPE_NAME,
            win32pipe.PIPE_ACCESS_DUPLEX,
            win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_READMODE_MESSAGE | win32pipe.PIPE_WAIT,
            win32pipe.PIPE_UNLIMITED_INSTANCES,
            65536, 65536,
            0,
            security_attributes,
        )

    def monitor_directory(self, directory, extension):
        result_files = list()  # Список для збереження результатів
        total_size = 0
        try:
            for root, _, files in os.walk(directory):  # Локальна змінна `files` з імен файлів
                for file in files:
                    if file.endswith(extension):
                        file_path = os.path.join(root, file)
                        size = os.path.getsize(file_path)
                        creation_time = os.path.getmtime(file_path)
                        result_files.append({  # Додаємо до `result_files`
                            "name": file,
                            "size": size,
                            "creation_time": datetime.datetime.fromtimestamp(creation_time).strftime("%Y-%m-%d %H:%M")
                        })
                        total_size += size
            print({"total_size": total_size, "files": result_files})
            return {"total_size": total_size, "files": result_files}
        except Exception as e:
            return {"error": str(e)}

    def process_request(self, request):
        print(request)
        directory = request.get("directory")
        extension = request.get("extension")
        if extension:
            extension = extension.replace("\\", "\\\\")
            extension = extension.replace("/", "\\\\")
        print("directory:", directory)
        print("extension", extension)

        if not directory or not extension:
            return {"error": "Invalid request: 'directory' and 'extension' are required"}

        cache_key = f'{directory}:{extension}'
        current_time = time.time()

        with self.lock:
            if cache_key in self.cache and (current_time - self.cache_life_stamp[cache_key] < CACHE_LIFE_TIME):
                return self.cache[cache_key]

            result = self.monitor_directory(directory, extension)
            self.cache[cache_key] = result
            self.cache_life_stamp[cache_key] = current_time
        return result

    # Обробка запита клієнта
    def handle_client(self, pipe):
        try:
            while True:
                result, data = win32file.ReadFile(pipe, 65536)
                request = json.loads(data.decode('utf-8'))

                response = self.process_request(request)

                win32file.WriteFile(pipe, json.dumps(response).encode('utf-8'))
        except Exception as e:
            print(f'Client disconnected or error occurred: {e}')
        finally:
            win32file.CloseHandle(pipe)

    def run(self):
        print(f'Server started on PIPE {PIPE_NAME}')
        while True:
            pipe = self.create_pipe()
            print('Waiting for connection...')
            win32pipe.ConnectNamedPipe(pipe, None)
            print('Connected.')

            client_thread = threading.Thread(target=self.handle_client, args=(pipe,))
            client_thread.start()


if __name__ == "__main__":
    server = FileMonitorServer()
    server.run()
