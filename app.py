from flask import Flask, request, send_file, jsonify, render_template
from Crypto.Cipher import AES
from Crypto.Util.Padding import pad, unpad
import TEErandom
import base64
import os
import tempfile

app = Flask(__name__)

# 密钥长度，AES-256需要32字节
KEY_SIZE = 32
IV_SIZE = 16


@app.route('/')
def index():
    return render_template('index.html')

@app.route('/upload', methods=['POST'])
def upload_file():
    file = request.files['file']
    if file:
        # 生成随机密钥
        key = TEErandom.generate_random(KEY_SIZE)
        iv = TEErandom.generate_random(IV_SIZE)
        # 初始化加密器
        cipher = AES.new(key, AES.MODE_CBC, iv)
        # 读取文件内容并加密
        file_data = file.read()
        encrypted_data = cipher.encrypt(pad(file_data, AES.block_size))
        # 将密钥和加密后的文件数据转换为base64
        key_b64 = base64.b64encode(key + iv).decode('utf-8')
        file_b64 = base64.b64encode(encrypted_data).decode('utf-8')
        # 获取文件名并编码为base64 (使用UTF-8编码)
        filename_b64 = base64.b64encode(file.filename.encode('utf-8')).decode('utf-8')
        # 将加密文件、密钥和文件名发送给客户端
        return jsonify({'key': key_b64, 'file': file_b64, 'filename': filename_b64}), 200
    return jsonify({'error': 'No file provided'}), 400

@app.route('/decrypt', methods=['POST'])
def decrypt_file():
    key_b64 = request.form['key']
    file = request.files['file']
    if key_b64 and file:
        # 将base64密钥和文件数据解码
        key = base64.b64decode(key_b64)[:32]
        iv = base64.b64decode(key_b64)[32:]
        encrypted_data = file.read()
        # 初始化解密器
        cipher = AES.new(key, AES.MODE_CBC, iv)
        # 解密文件数据
        decrypted_data = unpad(cipher.decrypt(encrypted_data), AES.block_size)
        # 将解密后的文件数据转换为base64
        file_b64 = base64.b64encode(decrypted_data).decode('utf-8')
        
        # 发送文件给客户端下载
        return jsonify({'file': file_b64}), 200
    return jsonify({'error': 'Missing key or file'}), 400

if __name__ == '__main__':
    app.run()
