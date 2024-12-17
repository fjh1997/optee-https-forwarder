import socket
import _thread as thread
import Tsecurestorage
from OpenSSL import SSL, crypto

# 设置 pyOpenSSL 的 SSL 上下文
cert_and_key = Tsecurestorage.read_secure_object("cert_and_key").decode()
# 获取临时证书和密钥路径
cert_pem = cert_and_key[:2008]
private_key_pem = cert_and_key[2008:]
# 加载证书和私钥
cert = crypto.load_certificate(crypto.FILETYPE_PEM, cert_pem)
pkey = crypto.load_privatekey(crypto.FILETYPE_PEM, private_key_pem)

# 创建 SSL 上下文
context = SSL.Context(SSL.TLSv1_2_METHOD)
context.use_certificate(cert)
context.use_privatekey(pkey)


def forward(source, destination):
    try:
        string = ' '
        while string:
            string = source.recv(1024)
            if string:
                destination.sendall(string)
            else:
                source.shutdown(socket.SHUT_RD)
                destination.shutdown(socket.SHUT_WR)
    except Exception as e:
        print(f"数据转发时出错: {e}")


dock_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
dock_socket.bind(('', 443))
dock_socket.listen(5)

while True:
    try:
        client_socket, client_address = dock_socket.accept()
        print(f"新连接: {client_address}")
        
        ssl_conn = SSL.Connection(context, client_socket)
        ssl_conn.set_accept_state()
        
        try:
            ssl_conn.do_handshake()
        except SSL.Error as e:
            # 检查错误消息是否与特定的 SSL 错误匹配
            error_msg = str(e)
            if 'ssl/tls alert certificate unknown' in error_msg:
                print(f"SSL 错误: 未知证书警告 - {error_msg}")
            elif 'http request' in error_msg:
                print(f"SSL 错误: HTTP 请求错误 - {error_msg}")
            else:
                print(f"SSL 握手错误: {error_msg}")
            ssl_conn.close()
            continue
    #    print("aaaaa")
        # 连接后端服务器
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.connect(('localhost', 5000))
        
        # 启动转发线程
        thread.start_new_thread(forward, (ssl_conn, server_socket))
        thread.start_new_thread(forward, (server_socket, ssl_conn))
    except Exception as e:
        print(f"处理连接时出错: {e}")
