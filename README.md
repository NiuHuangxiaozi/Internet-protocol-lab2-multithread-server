# readme

### 程序的构成：

```
--readme.md //程序的整体说明
--main.cpp //客户端程序的主程序
--show.cpp //客户端程序的一些状态转化函数，具体行为
--show.h //main.cpp和show.cpp的头文件，存放协议结构体
--makefile //makefile文件，make来进行编译
--server.cpp //服务器端的主程序
--server_show.cpp //服务器端的具体操作，一些状态转化函数
--server_show.h ////server_show.cpp和server.cpp的头文件，存放协议结构体
```

### 程序的运行：

```
1、进入程序所在文件夹;
2、make clean
3、 make
4、./server //启动服务器端
5、./main  //启动客户端
```

