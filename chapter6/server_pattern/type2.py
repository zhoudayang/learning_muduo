# __author__ = 'fit'
# -*- coding: utf-8 -*-
from SocketServer import BaseRequestHandler, TCPServer
from SocketServer import ForkingTCPServer, ThreadingTCPServer


#  传统Java网络编程方案，每个连接使用一个线程来处理，称为thread-per-connection
class EchoHandler(BaseRequestHandler):
    def handle(self):
        print "got connection from", self.client_address
        while True:
            data = self.request.recv(4096)
            if data:
                print "the client says ", data
                sent = self.request.send(data)
            else:
                print "disconnect", self.client_address
                self.request.close()
                break


if __name__ == "__main__":
    listen_address = ("0.0.0.0", 2016)
    # 相比type1,改动在此
    server = ThreadingTCPServer(listen_address, EchoHandler)
    server.serve_forever()