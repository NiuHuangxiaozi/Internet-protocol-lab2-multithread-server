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
  else if (operation[0] >= '0' && operation[0] <= '4')
    return;
  else
    throw operation;
}
union Client_Buffet cb;          // send buffer
union Server_Buffet interupt_sb; // only used for receive interrupted message

enum user_state
{
  LOGIN_IN = 0,
  BASE_UI = 1,
  READY_BATTLE = 2,
  COMBATING = 3
};
string operation;                 // basic ui operations
char Global_user_name[NAME_SIZE]; // USER's name

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
//
void show_login_client(union Server_Buffet *sb, int sockfd); // only for read
//
void look_for_battles_read(union Server_Buffet *sb, int sockfd);
void look_for_battles_write(int sockfd);
//
void Ready_for_battle_write(int sockfd); // ready for battle
//
void Fighting_write(int sockfd); // in fight ,send message
//
void Fight_read(union Server_Buffet *sb, int sockfd); // in fight ,receive message
//

// state functions
int state = 0; // user initial state
void normal_action_read(union Server_Buffet *sb, int sockfd)
{
  // cout << "normal action read" << endl;
  //  receive one client dump whenever what you do
  if (int(sb->content.operation_number) == CLIENT_LOGIN)
  {
    show_login_client(sb, sockfd); // only for read
    string watchout = "just look";
    throw watchout;
  }
  else if (int(sb->content.operation_number) == CLIENT_EXIT && int(sb->content.counter_part_dump) == 0)
  {
    show_exit_client(sb, sockfd); // only for read
    string watchout = "just look";
    throw watchout;
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
    case READY_BATTLE:
      look_for_battles_read(sb, sockfd);
      break;
    case COMBATING:
      Fight_read(sb, sockfd);
      break;
    default:
      break;
    };
  }
}
void normal_action_write(int sockfd)
{
  memset(cb.characters, 0, sizeof(cb.characters));
  switch (state)
  {
  case LOGIN_IN: // login stage
    Login_stage_write(sockfd);
    break;
  case BASE_UI:
    Basic_Ui_write(sockfd);
    break;
  case READY_BATTLE:
    Ready_for_battle_write(sockfd);
    break;
  case COMBATING:
    Fighting_write(sockfd);
    break;
  default:
    break;
  };
}

void Login_stage_read(union Server_Buffet *sb, int sockfd)
{
  if (int(sb->content.login_state) == LOGIN_STATE_SUCCESS)
  {
    cout << "Login in successfully" << endl;
    memset(Global_user_name, 0, sizeof(Global_user_name));
    strcpy(Global_user_name, sb->content.user_name);
    Global_user_name[NAME_SIZE - 1] = '\0';
    state = BASE_UI;
  }
  else if (int(sb->content.login_state) == LOGIN_PLAYER_FULL)
  {
    system("clear");
    cout << "The game is full of players" << endl;
    cout << "please press any button to exit!" << endl;
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

  memset(cb.content.user_name, 0, NAME_SIZE);
  cin >> cb.content.user_name;
  cb.content.user_name[NAME_SIZE - 1] = '\0';

  cout << "user password " << endl;
  cin >> cb.content.user_password;
  cb.content.operation_number = LOGIN_TEST;
  int send_flag = send(sockfd, cb.characters, sizeof(cb.characters), 0);
  if (send_flag <= 0)
    throw 1;
}

void Receive_Fight(union Server_Buffet *sb, int sockfd)
{
  state = READY_BATTLE;
  memset(interupt_sb.content.peer_name, 0, sizeof(interupt_sb.content.peer_name));
  strcpy(interupt_sb.content.peer_name, sb->content.peer_name);
  interupt_sb.content.peer_name[sizeof(interupt_sb.content.peer_name) - 1] = '\0';
}
void Basic_Ui_read(union Server_Buffet *sb, int sockfd)
{
  int opera = operation[0] - '0';
  if (int(sb->content.operation_number) == RESPONSE_BATTLE)
  {
    cout << "Receive fight" << endl;
    Receive_Fight(sb, sockfd);
  }
  else
  {
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
    if (state == BASE_UI)
      ask_main_information_write(sockfd);
    else
    {
      cout << "You have just received a invitation! show as follows" << endl;
      throw 2; // continue avoid wait
    }
    break;
  case 1:
    if (state == BASE_UI)
      look_for_rivals_write(sockfd);
    else
    {
      cout << "You have just received a invitation! show as follows" << endl;
      throw 2; // continue avoid wait
    }
    break;
  case 2:
    if (state == BASE_UI)
      exit_from_platform(sockfd);
    else
    {
      cout << "You have just received a invitation! show as follows" << endl;
      throw 2; // continue avoid wait
    }
    break;
  case 3:
    table_clear(sockfd);
    break;
  case 4:
    look_for_battles_write(sockfd);
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
  cout << "(State 1 :Free || State 2:Ready || State 3:Combating)" << endl;
  cout << "============================================" << endl;
}
void ask_main_information_write(int sockfd)
{
  cb.content.operation_number = ASK_MAIN_INFORMATION;
  memset(cb.content.user_name, 0, sizeof(cb.content.user_name));
  strcpy(cb.content.user_name, Global_user_name);
  cb.content.user_name[sizeof(cb.content.user_name) - 1] = '\0';
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

void show_exit_client(union Server_Buffet *sb, int sockfd) // only for read
{
  cout << endl
       << "A player just went offline ! ( Name : " << sb->content.user_name << " )"
       << "State[" << int(sb->content.state) << "]" << endl;
}

void show_login_client(union Server_Buffet *sb, int sockfd) // only for read
{
  cout << endl
       << "A player just went online ! ( Name : " << sb->content.user_name << " )"
       << "State[" << int(sb->content.state) << "]" << endl;
}

//
void look_for_battles_read(union Server_Buffet *sb, int sockfd)
{
  if (int(sb->content.operation_number) == CLIENT_EXIT && int(sb->content.counter_part_dump) == 1)
  {
    cout << "The counterpart  has dumped!" << endl;
    cout << "Press any button to continue" << endl;
    state = BASE_UI;
    return;
  }
  // normal
  int error_code = int(sb->content.find_rival_error_code);
  switch (error_code)
  {
  case USER_AGREE_BATTLE:
    cout << "Rival agrees to fight with you!" << endl;
    state = COMBATING;
    break;
  case USER_NOT_EXIT:
    cout << "User not Exist!" << endl;
    state = BASE_UI;
    break;
  case USER_NOT_FREE:
    cout << "Rival are not free!" << endl;
    state = BASE_UI;
    break;
  case PEER_REFUSED:
    cout << endl
         << "The counterpart refuse your invitation!" << endl;
    state = BASE_UI;
    break;
  default:
    break;
  }
}
void look_for_battles_write(int sockfd)
{
  if (state == READY_BATTLE)
  {
    cout << "You are already in ready" << endl;
    throw 2; // 2 in main.cpp continue
  }
  cb.content.operation_number = SEARCH_BATTLE;
  char name[NAME_SIZE];
  while (true)
  {
    memset(name, 0, NAME_SIZE);
    cout << endl
         << "Please input the rival's name : ";
    cin >> name;
    name[NAME_SIZE - 1] = '\0';
    if (strcmp(name, Global_user_name) == 0)
    {
      cout << "Can't fight with yourself!" << endl;
    }
    else
      break;
  }
  memset(cb.content.rival_name, 0, NAME_SIZE); // rival's name
  strcpy(cb.content.rival_name, name);

  memset(cb.content.user_name, 0, NAME_SIZE); // my name
  strcpy(cb.content.user_name, Global_user_name);

  int send_flag = send(sockfd, cb.characters, sizeof(cb.characters), 0);
  assert(send_flag > 0);
  state = READY_BATTLE; // SWITCH to ready state
  cout << endl
       << "Please wait patiently" << endl;
}

void Ready_for_battle_write(int sockfd)
{
  string op;
  cout << "The player " << interupt_sb.content.peer_name << " want to fight with you! Do you agree?" << endl;
  cout << "1 means agree ,2 means  refuse" << endl;
  while (true)
  {
    cin >> op;
    if (op.size() != 1 || (op[0] != '1' && op[0] != '2'))
    {
      cout << "input error" << endl;
      continue;
    }
    break;
  }
  cb.content.operation_number = RESPONSE_BATTLE;

  memset(cb.content.rival_name, 0, sizeof(cb.content.rival_name));
  strcpy(cb.content.rival_name, interupt_sb.content.peer_name);
  cb.content.rival_name[sizeof(cb.content.rival_name) - 1] = '\0';
  memset(cb.content.user_name, 0, sizeof(cb.content.user_name));
  strcpy(cb.content.user_name, Global_user_name);
  cb.content.user_name[sizeof(cb.content.user_name) - 1] = '\0';

  if (op == "1") // agree
  {
    cb.content.whether_accept = ACCEPT_FIGHT;
    int send_flag = send(sockfd, cb.characters, sizeof(cb.characters), 0);
    assert(send_flag > 0);
    state = COMBATING; // reback to fighting
    throw 2;
  }
  else // refuse
  {
    cb.content.whether_accept = REFUGE_FIGHT;
    int send_flag = send(sockfd, cb.characters, sizeof(cb.characters), 0);
    assert(send_flag > 0);
    state = BASE_UI; // reback to mian ui
    throw 2;         // [continue] after refuse others just reback to main ui.
  }
}

void Fighting_write(int sockfd)
{
  string op;
  cout << "Please input your choice !" << endl;
  cout << "0 : stone |  1 : scissors |  2 : cloth" << endl;
  while (true)
  {
    cin >> op;
    if (op.size() != 1 || (op[0] != '0' && op[0] != '1' && op[0] != '2'))
    {
      cout << "input error" << endl;
      continue;
    }
    break;
  }
  if (state == COMBATING)
  {
    // send your choice add operation number ,name and choice
    cb.content.operation_number = FIGHTING_STATE;
    memset(cb.content.user_name, 0, sizeof(cb.content.user_name));
    strcpy(cb.content.user_name, Global_user_name);
    cb.content.user_name[sizeof(cb.content.user_name) - 1] = '\0';
    cb.content.choice = int(op[0] - '0');
    //
    int send_flag = send(sockfd, cb.characters, sizeof(cb.characters), 0);
    assert(send_flag > 0);
    cout << "Waiting for compete Answer.................." << endl;
  }
  else
  {
    throw 2; // ignore wait ,directly turn to base ui
  }
}
void Fight_read(union Server_Buffet *sb, int sockfd)
{
  // counterpart dump
  if (int(sb->content.operation_number) == CLIENT_EXIT && int(sb->content.counter_part_dump) == 1)
  {
    cout << "The counterpart has dumped!" << endl;
    state = BASE_UI;
    return;
  }
  // normal
  assert(int(sb->content.operation_number) == FIGHTING_STATE);
  if (sb->content.win_state == EQUAL)
  {
    cout << "The rival choose the  same gesture with you!" << endl;
  }
  else if (int(sb->content.win_state) == WINONCE)
  {
    cout << "Rival :" << sb->content.peer_name << " | Choice : " << int(sb->content.peer_name_choice) << endl;
    cout << "You :" << sb->content.user_name << " | Choice : " << int(sb->content.user_name_choice) << endl;
    cout << "You win in this round !  The current blood of rival : " << int(sb->content.blood) << endl;
  }
  else if (int(sb->content.win_state) == LOSEONCE)
  {
    cout << "Rival :" << sb->content.peer_name << " | Choice : " << int(sb->content.peer_name_choice) << endl;
    cout << "You :" << sb->content.user_name << " | Choice : " << int(sb->content.user_name_choice) << endl;
    cout << "You lose in this round !  The current blood of rival : " << int(sb->content.blood) << endl;
  }
  else if (int(sb->content.win_state) == FINALWIN)
  {
    cout << "Congratulations! You have defeated the opponent [ " << sb->content.peer_name << " ] in this game !" << endl;
    state = BASE_UI;
  }
  else if (int(sb->content.win_state) == FINALLOSE)
  {
    cout << "What a pity! You have been defeated by the opponent [ " << sb->content.peer_name << " ] in this game !" << endl;
    state = BASE_UI;
  }
}