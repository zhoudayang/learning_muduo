# -*- coding:utf-8 -*- 
# author：zhouyang
import socket


# 最基本的服务器,一次只能服务一个客户端,服务完释放连接才能继续服务下一个客户端
def handle(client_socket, client_address):
    while True:
        data = client_socket.recv(4096)
        if data:
            print 'the client says ', data
            sent = client_socket.send(data)
        else:
            print "disconnect", client_address
            client_socket.close()
            break


if __name__ == "__main__":
    listen_address = ("0.0.0.0", 2016)
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(listen_address)
    server_socket.listen(5)
    while True:
        (client_socket, client_address) = server_socket.accept()
        print "got connection from", client_address
        handle(client_socket, client_address)
