# -*- coding:utf-8 -*- 
# author：zhouyang

import socket
import threading


class myClient(threading.Thread):
    def __init__(self, address, port):
        threading.Thread.__init__(self)
        self.address = address
        self.port = port

    def run(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        address_info = (self.address, self.port)
        sock.connect(address_info)
        hello_message="hello server! I am client"
        sock.send(hello_message)
        message = sock.recv(4096)
        print message
        sock.close()


if __name__ == "__main__":
    thread_list = []
    for i in range(10):
        thread_list.append(myClient("127.0.0.1", 2016))
    for one_thread in thread_list:
        one_thread.start()
