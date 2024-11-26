import os
import time
import datetime
import json
import win32pipe
import win32file
import pywintypes
from collections import defaultdict
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
        security_descriptor.Initialize(security_attributes)
        security_descriptor.SetSecurityDescriptorDacl(1, None, 0)
        security_attributes.SecurityDescriptor = security_descriptor

        return win32pipe.CreatePipe(
            PIPE_NAME,
            win32pipe.PIPE_ACCESS_DUPLEX,
            win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_READMODE_MESSAGE | win32pipe.PIPE_WAIT,
            win32pipe.PIPE_UNLIMITED_INSTANCES,
            65536, 65536,
            0,
            security_attributes
        )

    def process_request(self, request):
        directory = request.get("directory")
        extension = request.get("extension")

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
            pipe = win32pipe.ConnectNamedPipe(
                PIPE_NAME,
                win32pipe.PIPE_ACCESS_DUPLEX,
                win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_READMODE_MESSAGE | win32pipe.PIPE_WAIT,
                win32pipe.PIPE_UNLIMITED_INSTANCES,
                65536, 65536,
                0,
                None
            )

            print('Waiting for connection...')
            win32pipe.ConnectNamedPipe(pipe, None)
            print('Connected.')

            client_thread = threading.Thread(target=self.handle_client, args=(pipe,))
            client_thread.start()


if __name__ == "__main__":
    server = FileMonitorServer()
    server.run()
