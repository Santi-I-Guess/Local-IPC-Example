import os
import socket
import time

with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as client_sock:
    
    client_sock.connect("/tmp/something.sock")

    while True:
        buffer = input("outgoing client message: ")
        if len(buffer) > 127:
            print("buffer too long for server to handle")
            break
    
        client_sock.send(buffer.encode())
        time.sleep(1)

        message_given = client_sock.recv(128).decode()
        message_given.replace("\n", "\0")
        print(f"recieved from server: {message_given}")
