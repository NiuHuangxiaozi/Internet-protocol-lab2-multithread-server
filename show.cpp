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
  cout << "4.look for matchmaking" << endl;
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
  BASE_UI = 1,
};
string operation; // basic ui operations

// some send and recv functions
void Basic_Ui_read(union Server_Buffet *sb, int sockfd);
void Basic_Ui_write(int sockfd);
//
void Login_stage_read(union Server_Buffet *sb, int sockfd);
void Login_stage_write(int sockfd);
//
void ask_main_information_read(union Server_Buffet *sb, int sockfd);
void ask_main_information_write(int sockfd);
//
void exit_from_platform(int sockfd); // only for write
//
void look_for_rivals_read(union Server_Buffet *sb, int sockfd);
void look_for_rivals_write(int sockfd);
//
void table_clear(int sockfd); // only for write (clear)
//
void show_exit_client(union Server_Buffet *sb, int sockfd); // only for read

// state functions
int state = 0; // user initial state
void normal_action_read(union Server_Buffet *sb, int sockfd)
{
  // receive one client dump whenever what you do
  if (int(sb->content.operation_number) == CLIENT_EXIT)
  {
    show_exit_client(sb, sockfd); // only for read
  }
  else
  {
    switch (state)
    {
    case LOGIN_IN: // login stage
      Login_stage_read(sb, sockfd);
      break;
    case BASE_UI:
      Basic_Ui_read(sb, sockfd);
      break;
    };
  }
}
void normal_action_write(int sockfd)
{
  switch (state)
  {
  case LOGIN_IN: // login stage
    Login_stage_write(sockfd);
    break;
  case BASE_UI:
    Basic_Ui_write(sockfd);
    break;
  };
}

void Login_stage_read(union Server_Buffet *sb, int sockfd)
{
  if (int(sb->content.login_state) == LOGIN_STATE_SUCCESS)
  {
    cout << "Login in successfully" << endl;
    state = BASE_UI;
  }
  else if (int(sb->content.login_state) == LOGIN_PLAYER_FULL)
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
void Login_stage_write(int sockfd)
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
}
void Basic_Ui_read(union Server_Buffet *sb, int sockfd)
{
  int opera = operation[0] - '0';
  switch (opera)
  {
  case 0: // look for main information
    ask_main_information_read(sb, sockfd);
    break;
  case 1:
    look_for_rivals_read(sb, sockfd);
    break;
  default:
    break;
  }
}

void Basic_Ui_write(int sockfd)
{
  print_main_ui();
  // examination for input order
  cout << "input operation : ";
  while (true)
  {
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
    break;
  }
  // do the job
  int opera = operation[0] - '0';
  switch (opera)
  {
  case 0: // look for main information
    ask_main_information_write(sockfd);
    break;
  case 1:
    look_for_rivals_write(sockfd);
    break;
  case 2:
    exit_from_platform(sockfd);
    break;
  case 3:
    table_clear(sockfd);
    break;
  default:
    break;
  }
}

// 0
void ask_main_information_read(union Server_Buffet *sb, int sockfd)
{
  cout << "Personal Main Information" << endl;
  cout << "Name: " << sb->content.user_name << endl;
  cout << "Passward: " << sb->content.user_password << endl;
  cout << "Blood: " << int(sb->content.blood) << endl;
  cout << "State: " << int(sb->content.state) << endl;
  cout << "(State 0 :Free || State 1:Ready || State 2:Combating)" << endl;
  cout << "============================================" << endl;
}
void ask_main_information_write(int sockfd)
{
  cb.content.operation_number = ASK_MAIN_INFORMATION;
  int send_flag = send(sockfd, cb.characters, sizeof(cb.characters), 0);
  assert(send_flag > 0);
}
// 1
void look_for_rivals_read(union Server_Buffet *sb, int sockfd)
{
  int number = int(sb->content.player_number);
  cout << number << endl;
  cout << "All Players" << endl;
  cout << "=========================================" << endl;
  int current_state = -1;
  for (int index = 0; index < number; index++)
  {
    cout << "Player [" << index << "] ";
    string name;
    for (int j = 0; j < 10; j++)
      name += sb->content.members[index * 10 + j];
    current_state = int(sb->content.members[(index + 1) * 10]);
    cout << "Name : " << name << "   "
         << "State : " << current_state << endl;
  }
  cout << "=========================================" << endl;
  cout << "[ 0 :Free  1:Ready  2: Combat ]" << endl;
}
void look_for_rivals_write(int sockfd)
{
  cb.content.operation_number = LOOK_FOR_RIVALS;
  int send_flag = send(sockfd, cb.characters, sizeof(cb.characters), 0);
  assert(send_flag > 0);
}
// 2
void exit_from_platform(int sockfd)
{
  shutdown(sockfd, SHUT_WR);
  throw 1;
}
void table_clear(int sockfd)
{
  system("clear");
  // print_main_ui();
  throw 2;
}

void show_exit_client(union Server_Buffet *sb, int sockfd)
{
  cout << "A player just went offline ! ( " << sb->content.user_name << " )"
       << "State[" << int(sb->content.state) << "]" << endl;
}