import threading
import socket
import queue

class Client:
    def __init__(self, server_address, server_port):
        self.server_address = server_address
        self.server_port = server_port
        self.running = True
        self.sock = None
        self.message_queue = queue.Queue()
        self.receive_thread = None

    def connect(self):
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((self.server_address, self.server_port))
            self.running = True
            
            self.receive_thread = threading.Thread(target=self._receive_messages, daemon=True)
            self.receive_thread.start()
            return True
        except Exception as e:
            print(f"Connection error: {e}")
            return False

    def _receive_messages(self):
        buffer = ""
        while self.running:
            try:
                data = self.sock.recv(1024)
                if not data:
                    break
                buffer += data.decode('utf-8')
                
                while '\n' in buffer:
                    line, buffer = buffer.split('\n', 1)
                    if line.strip():
                        self.message_queue.put(line.strip())
            except Exception as e:
                if self.running:
                    print(f"Receive error: {e}")
                break

    def send_message(self, message):
        if self.sock:
            try:
                self.sock.send((message + '\n').encode('utf-8'))
                return True
            except Exception as e:
                print(f"Send error: {e}")
                return False
        return False

    def get_message(self, timeout=0.1):
        try:
            return self.message_queue.get(timeout=timeout)
        except queue.Empty:
            return None

    def stop(self):
        self.running = False
        if self.sock:
            try:
                self.sock.close()
            except:
                pass
        