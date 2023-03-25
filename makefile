CC = g++ # 设置C++编译器
CFLAGS = -Wall # 添加编译标志

# 定义目标和依赖关系
all: main server

main: main.o show.o
	$(CC) $(CFLAGS) -o main main.o show.o

main.o: main.cpp show.h
	$(CC) $(CFLAGS) -c main.cpp

show.o: show.cpp show.h
	$(CC) $(CFLAGS) -c show.cpp

server: server.o server_show.o
	$(CC) $(CFLAGS) -o server server.o server_show.o

server.o: server.cpp server_show.h
	$(CC) $(CFLAGS) -c server.cpp

server_show.o: server_show.cpp server_show.h
	$(CC) $(CFLAGS) -c server_show.cpp

clean:
	rm -f *.o main server # 删除所有对象文件与可执行文件
