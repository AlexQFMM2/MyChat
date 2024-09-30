# ChatServer

已经处理好了服务器的搭建，利用epoll多路复用，轮询优化性能

在处理工作的时候通过线程池来分配线程处理来自客户端的请求

客户端和服务器端的互相传输通过JSON格式信息传输



执行方法

```
build目录下
./ChatServer <port>
```



ChatClient

目前只有命令行模拟，暂无qt实现



目前的代码仅仅只是连接，具体功能暂未实现

redis 安装

sudo apt update
sudo apt install redis-server
sudo systemctl start redis-server
sudo systemctl enable redis-server
redis-cli ping

Apache Kafka 安装
sudo apt install openjdk-11-jdk
wget https://downloads.apache.org/kafka/3.8.0/kafka_2.13-3.8.0.tgz
tar -xzf kafka_2.13-3.8.0.tgz
cd kafka_2.13-3.8.0
bin/zookeeper-server-start.sh config/zookeeper.properties
bin/kafka-server-start.sh config/server.properties

rabbitmq 安装
sudo apt install rabbitmq-server
sudo systemctl start rabbitmq-server
sudo systemctl enable rabbitmq-server
sudo rabbitmqctl status
sudo rabbitmq-plugins enable rabbitmq_management
