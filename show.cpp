#include "show.h"

const char *tar_ip = "127.0.0.1";
int tar_port = 4321;

// ui
void print_login_ui()
{
  cout << "Welcome to NJUCS Play Program!" << endl;
  cout << "Please input your name and password to login in" << endl;
}
void print_not_connection()
{
  cout << "Can Not Connect To Server!" << endl;
}

void print_main_ui()
{

  cout << "===================Main Table===================" << endl;
  cout << "0.Look for basic information" << endl;
  cout << "1.Look for rivals" << endl;
  cout << "2.back" << endl;
  cout << "3.clear" << endl;
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
void test_operation(string operation)
{
  if (operation.size() == 0 || operation.size() >= 2)
    throw operation;
  else if (operation[0] >= '0' && operation[0] <= '3')
    return;
  else
    throw operation;
}
union Client_Buffet cb; // send buffer
union Server_Buffet sb; // receive buffer

enum user_state
{
  LOGIN_IN = 0,
  BASE_UI = 1
};

// some send and recv functions
void Basic_Ui(int sockfd);
void Login_stage(int sockfd);
void ask_main_information(int sockfd);

//
// state functions
int state = 0; // user initial state
void normal_action(int sockfd)
{
  switch (state)
  {
  case LOGIN_IN: // login stage
    Login_stage(sockfd);
    break;
  case BASE_UI:
    Basic_Ui(sockfd);
    break;
  };
}

void Login_stage(int sockfd)
{
  print_login_ui();
  cout << "user name" << endl;
  cin >> cb.content.user_name;
  cout << "user password " << endl;
  cin >> cb.content.user_password;
  cb.content.operation_number = LOGIN_TEST;

  int send_flag = send(sockfd, cb.characters, sizeof(cb.characters), 0);
  if (send_flag <= 0)
    throw 1;
  int recv_flag = recv(sockfd, sb.characters, sizeof(sb.characters), 0);
  if (recv_flag <= 0)
    throw 1;
  if (int(sb.content.login_state) == LOGIN_STATE_SUCCESS)
  {
    cout << "Login in successfully" << endl;
    system("clear");
    state = BASE_UI;
  }
  else if (int(sb.content.login_state) == LOGIN_PLAYER_FULL)
  {
    system("clear");
    cout << "The game is full of players" << endl;
    throw 2;
  }
  else
  {
    system("clear");
    cout << "Login in fail" << endl;
  }
}

void Basic_Ui(int sockfd)
{
  print_main_ui();
  // examination for input order
  string operation;
  while (true)
  {
    cout << "input operatopn: ";
    cin >> operation;
    try
    {
      test_operation(operation);
    }
    catch (string s)
    {
      print_input_error();
      continue;
    }
    // do the job
    int opera = operation[0] - '0';
    switch (opera)
    {
    case 0: // look for main information
      ask_main_information(sockfd);
      break;
    case 1:
      cout << "case 1" << endl;
      break;
    case 2:
      cout << "case 2" << endl;
      break;
    case 3:
      cout << "case 3" << endl;
      system("clear");
      print_main_ui();
      break;
    default:
      break;
    }
  }
}

void ask_main_information(int sockfd)
{
  cb.content.operation_number = ASK_MAIN_INFORMATION;
  int send_flag = send(sockfd, cb.characters, sizeof(cb.characters), 0);
  if (send_flag <= 0)
    return;
  int recv_flag = recv(sockfd, sb.characters, sizeof(sb.characters), 0);
  if (recv_flag <= 0)
    return;
  cout << "recevice main information" << endl;
  cout << "Name: " << sb.content.user_name << endl;
  cout << "Passward: " << sb.content.user_password << endl;
  cout << "Blood: " << int(sb.content.blood) << endl;
  cout << "State: " << int(sb.content.state) << endl;
  cout << "(State 0 :Free || State 1:Ready || State 2:Combating)" << endl;
  cout << "=========================================" << endl;
}
