# __author__ = 'fit'
# -*- coding: utf-8 -*-
import socket
import select

# 基本的单线程Reactor方案
# 由网络库搞定数据收发，程序只关心业务逻辑
# 适合IO密集的应用，不太适合cpu密集的应用，较难发挥多核的威力
# 因为多了一次pool涉及的系统调用，处理网络消息的延迟会略大

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# 第二个参数level是被设置的选项的级别，如果想要在套接字上设置选项，就必须把level设置为SOL_SOCKET
# SO_REUSEADDR ，打开或关闭地址复用功能。 option_value为１，代表打开地址复用功能
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_socket.bind(('', 2016))
# it specifies the number of unaccepted connections that the system will allow before refusing newconnections.
server_socket.listen(5)
server_socket.setblocking(0)
poll = select.poll()
connections = {}
handlers = {}


def handler_input(socket, data):
    print "the client says ", data
    socket.send(data)


def handle_request(fileno, event):
    # 对于客户连接，读取并回显数据，并处理连接的关闭．
    if event & select.POLLIN:
        client_socket,client_address = connections[fileno]
        data = client_socket.recv(4096)
        if data:
            # 数据处理
            handler_input(client_socket,data)
        else:
            # 断开连接
            poll.unregister(fileno)
            client_socket.close()
            print "disconnect ",client_address
            del connections[fileno]
            del handlers[fileno]


# 接受新连接，并注册到IO事件关注列表，然后把连接添加到connections字典中．
def handle_accept(fileno, event):
    (client_socket, client_address) = server_socket.accept()
    print "got connecion from", client_address
    # client_socket.setblocking(0)
    poll.register(client_socket.fileno(), select.POLLIN)
    connections[client_socket.fileno()] = (client_socket,client_address)
    # 注册回调函数
    handlers[client_socket.fileno()] = handle_request

# 注册accpet事件
poll.register(server_socket.fileno(), select.POLLIN)
# 注册accpet事件的处理函数
handlers[server_socket.fileno()] = handle_accept

# 程序的核心是事件循环，事件的处理通过handlers转发到各个函数中，不再集中在一起
# listening fd的处理函数是handle_accept,它会注册客户连接的handler
# 普通客户链接的处理函数是handle_request，其中又把连接断开和数据到达这两个事件分开，后者由handle_input处理
# 业务逻辑位于单独的hanle_input函数中，实现了分离
while True:
    events = poll.poll(10000)
    for fileno, event in events:
        handler = handlers[fileno]
        handler(fileno, event)
