import threading
import socket

class Client(threading.Thread):
    def __init__(self, server_address, server_port):
        super().__init__()
        self.server_address = server_address
        self.server_port = server_port
        self.running = True

    def run(self):
      
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
                sock.connect((self.server_address, self.server_port))
                message = "Hello, Server!"
                sock.send(message.encode('utf-8'))
                response = sock.recv(1024)
                print(f"Received: {response.decode('utf-8')} xdxd")
        except Exception as e:
            print(f"An error occurred: {e}")
            

    def stop(self):
        self.running = False
        