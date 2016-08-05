# -*- coding:utf-8 -*-

import socket
import threading


class myClient(threading.Thread):
    # 初始化线程
    def __init__(self, address, port):
        threading.Thread.__init__(self)
        self.address = address
        self.port = port

    # 执行线程会调用这个函数
    def run(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        address_info = (self.address, self.port)
        sock.connect(address_info)
        sock.close()


if __name__ == "__main__":
    thread_list = []
    # 创建10个threading.Thread对象
    for i in range(1000):
        thread_list.append(myClient("127.0.0.1", 9987))
    # run
    for one_thread in thread_list:
        one_thread.start()