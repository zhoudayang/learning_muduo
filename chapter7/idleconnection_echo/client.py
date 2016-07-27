import socket
import time

sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
sock.connect(('127.0.0.1',2008))
sock.send("hello world\n")
time.sleep(20)
sock.send("hello world\n")
