# -*- coding:utf-8 -*- 
# author：zhouyang

import socket
import time

if __name__ == "__main__":
    sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    address_info = ("127.0.0.1",9981)
    sock.connect(address_info)
    sock.send("hello world from client!")
    sock.close()


