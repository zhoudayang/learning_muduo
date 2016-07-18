# __author__ = 'fit'
# -*- coding: utf-8 -*-
import socket
import select

# 设计模式４：采用IO复用

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# 第二个参数level是被设置的选项的级别，如果想要在套接字上设置选项，就必须把level设置为SOL_SOCKET
# SO_REUSEADDR ，打开或关闭地址复用功能。 option_value为１，代表打开地址复用功能
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_socket.bind(('', 2016))
server_socket.listen(5)
server_socket.setblocking(0)
poll = select.poll()
poll.register(server_socket.fileno(), select.POLLIN)
# 定义一个文件描述符到socket对象之间的的映射
connection = {}

# 程序主体是事件循环，
while True:
    events = poll.poll(10000)  # 10 seonds
    # 根据不同的描述符执行不同的操作
    for fileno, event in events:
        # 对于listen　fd,接受新连接，并注册到IO事件关注列表，然后把连接添加到connections字典中．
        if fileno == server_socket.fileno():
            (client_socket, client_address) = server_socket.accept()
            print "got connection from ", client_address
            poll.register(client_socket.fileno(), select.POLLIN)
            connection[client_socket.fileno()] = (client_socket, client_address)
        # 对于客户连接，读取并回显数据，并处理连接的关闭．
        elif event & select.POLLIN:
            client_socket = connection[fileno][0]
            data = client_socket.recv(4096)
            if data:
                print "the client says ", data
                client_socket.send(data)
            else:
                # 客户端断开了连接，将其从注册的事件中注销
                poll.unregister(fileno)
                client_socket.close()
                print "disconnect ", connection[fileno][1]
                del connection[fileno]
