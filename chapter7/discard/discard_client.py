# -*- coding:utf-8 -*- 
#author：zhouyang
import socket
import threading

class myClient(threading.Thread):
    def __init__(self,address,port):
        threading.Thread.__init__(self)
        self.address = address
        self.port = port
    def run(self):
        sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        address =(self.address,self.port)
        sock.connect(address)
        sock.send("hello world!")
        sock.close()


if __name__ == "__main__":
    thread_list = []
    for i in xrange(1000):
        thread_list.append(myClient("127.0.0.1",2009))
    for thread in thread_list:
        thread.start()
