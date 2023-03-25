#include "show.h"

const char *tar_ip = "127.0.0.1";
int tar_port = 4321;

// ui
void print_not_exist_city(string s)
{
  cout << "Sorry, Server does not have weather information for city " + s << endl;
}
void print_login_ui()
{
  cout << "Welcome to NJUCS Play Program!" << endl;
  cout << "Please input your name and password to login in" << endl;
}
void print_not_connection()
{
  cout << "Can Not Connect To Server!" << endl;
}

void print_whether_select()
{

  cout << "Please enter the given number to query" << endl;
  cout << "1.today" << endl;
  cout << "2.three days from today" << endl;
  cout << "3.custom day by yourself" << endl;
  cout << "(r)back,(c)cls,(#)exit" << endl;
  cout << "===================================================" << endl;
}
void print_input_error()
{
  cout << "input error!" << endl;
}

// 1程序启动的时候有无链接成功
int test_connection()
{
  int socket_state = socket(AF_INET, SOCK_STREAM, 0);
  assert(socket_state != -1);

  sockaddr_in servaddr;
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(tar_ip);
  servaddr.sin_port = htons(tar_port);

  int connect_state = connect(socket_state, (sockaddr *)&servaddr, sizeof(servaddr));

  if (connect_state == -1) // fail return -1
  {
    print_not_connection();
    throw connect_state;
  }

  // successful return socket number
  return socket_state;
}