# -*- coding:utf-8 -*-
import socket

sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
sock.connect(('127.0.0.1',2016))
# recevie data from server
data =sock.recv(4096)
print data
# send data to server
sock.send("hello world\n")
# close socket
sock.close()