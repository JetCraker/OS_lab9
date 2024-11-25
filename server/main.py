import os
import time
import datetime
import json
import win32pipe
import win32file
import pywintypes
from collections import defaultdict
import threading
from settings import CASH_LIFE_TIME, PIPE_NAME


class FileMonitorServer:
    def __init__(self):
        self.cash = dict()
        self.cash_life_stamp = dict()
        self.lock = threading.Lock()


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

            #TODO:Тут треба трохи доробити
            client_thread = threading.Thread(target=self.handle_client, args=(pipe,))
            client_thread.start()


if __name__ == "__main__":
    server = FileMonitorServer()
    server.run()
