# 基于optee的https转发器，转发为http
首届教育信创大赛未获奖作品之一<br>
首先Python从TEE环境里读取私钥，再把私钥与openSSL进行结合实现SSL通信，然后监听本地的443端口，使用SSL通信，与访问443端口的浏览器进行交互，再将交互后得到的信息通过本地转发的方式转发到本地的5000端口。
## 使用方法：
1.刷入带optee的系统<br>
2.编译python模块<br>
SaveStorage 下执行./build_storage 进行环境编译<br>
random下执行./build_random 进行环境编译<br>
3.开启web服务：
执行./start.sh 启动本地服务端
## Title: HTTPS Forwarder Based on OP-TEE, Forwarding to HTTP  
This project was one of the entries in the first Chinese Educational IT Innovation Competition but did not win an award.  
### Description  
The project uses Python to read a private key from the TEE (Trusted Execution Environment). This private key is then integrated with OpenSSL to establish SSL communication. The program listens on the local 443 port and interacts with browsers that access this port using SSL. After processing the interaction, the received data is locally forwarded to the HTTP server running on port 5000.  

### Usage Instructions:  
1. **Flash a system with OP-TEE**  
2. **Compile the Python modules**  
   - Navigate to the `SaveStorage` directory and execute `./build_storage` to compile the environment.  
   - Navigate to the `random` directory and execute `./build_random` to compile the environment.  
3. **Start the web service**  
   - Run `./start.sh` to launch the local server.
![image](https://github.com/user-attachments/assets/58ee0aa9-23f5-4742-b1ab-e6267878a16f)
![image](https://github.com/user-attachments/assets/c00502a1-7933-4a24-9be4-d58b743fabbe)
