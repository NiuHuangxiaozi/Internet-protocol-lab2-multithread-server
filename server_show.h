#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <map>
#include <vector>
#include <signal.h>
using namespace std;

// some configurations
#define SRV_PORT 4321           // server's port
#define MAX_CLIENT_CONNECTION 3 // max client connection contemporary
#define MAX_CLIENT 3            // max client to serve
#define TIME_INTEVAL 20         // 设置等待时间,select function wait inteval
#define NAME_SIZE 10            // name's size

// some resolve configurations

//  operation code
#define LOGIN_EXAMINATION 0
#define MAIN_INFORMATION 1
#define LOOK_FOR_RIVALS 2
#define CLIENT_EXIT 3
#define CLIENT_LOGIN 4
#define SEARCH_BATTLE 5
#define RESPONSE_BATTLE 6 // one is ready ask another
#define FIGHTING_STATE 7  // fighting code

// Client_Buffet symbols
#define LOGIN_STATE_FAIL 0x00    // login fail
#define LOGIN_STATE_SUCCESS 0x01 // login success
#define LOGIN_PLAYER_FULL 0x02   //  player full

#define USER_AGREE_BATTLE 0x0 // rival agree to battle
#define USER_NOT_EXIT 0x1     // search battle
#define USER_NOT_FREE 0x2     // search battle
#define PEER_REFUSED 0x3      // search battle

#define ACCEPT_FIGHT 1 // ACCEPT the fight (use in whether accept)
#define REFUGE_FIGHT 0 // refuse whether accept(use in whether accept)

#define EQUAL 0     //[win_state]
#define WINONCE 1   // [win_state]
#define LOSEONCE 2  // [win_state]
#define FINALWIN 3  //[win_state]
#define FINALLOSE 4 //[win_state]
//

// client send buffer construction
union Client_Buffet
{
  struct
  {
    char operation_number;  // explare how to resolve
    char user_name[10];     // client name (use for login )
    char user_password[10]; // client password (use for login)
    char rival_name[10];    // look for rival's name
    char whether_accept;    // whether accept another challenge
    char choice;            // cloth ,sicssors, stone
    char un_use[95];        //

  } content;
  char characters[128];
};
///
union Server_Buffet
{
  struct
  {
    char operation_number;      // explare how to resolve
    char login_state;           // whether login successful
    char user_name[10];         // client name (use for login )
    char user_password[10];     // client password (use for login)
    char blood;                 // client blood
    char state;                 // client state
    char player_number;         // player's number (max 3)
    char members[11 * 3];       // player's (name ,state)
    char find_rival_error_code; // name not exit or  find myself to battle 0 means normal
    char peer_name[10];         // require another peer
    char user_name_choice;
    char peer_name_choice;
    char win_state;  // 0 means equal ,1 means win once ,2 means final win
    char no_use[56]; // leave to use
  } content;
  char characters[128];
};

class Client
{
  enum States
  {
    Lazy = 0,
    Ready = 1,
    Combat = 2
  };

private:
  string name;
  string password;
  int blood;
  int state;
  int sockfd;

public:
  Client(string new_name, int sock, string new_passord)
  {
    name = new_name;
    password = new_passord;
    blood = rand() % 50 + 1; // random blood
    state = Lazy;
    sockfd = sock;
  }
  Client()
  {
    blood = rand() % 50 + 1; // random blood
    state = Lazy;
    sockfd = -1;
  }
  bool test_identity(string new_name)
  {
    return new_name == name;
  }
  int get_sockfd()
  {
    return sockfd;
  }
  string get_name() { return name; }
  string get_password() { return password; }
  int get_state() { return state; }
  int get_blood() { return blood; }
  void set_online() { state = 1; }
  void set_ready() { state = 2; }
  void set_combat() { state = 3; }
};

class Platform
{
private:
  string user_1_name;
  int user_1_blood;
  int sockfd_1;
  bool is_ready_1; // whether give gesture
  string user_2_name;
  int user_2_blood;
  int sockfd_2;
  bool is_ready_2;
  int one_ac = 5;
  int choice_1 = -1;
  int choice_2 = -1;

  string current_win_name;
  string current_lose_name;
  bool is_round_equal = false; // whether is equal

public:
  Platform()
  {
  }
  Platform(string _user_1_name, int _user_1_blood, int _sockfd_1,
           string _user_2_name, int _user_2_blood, int _sockfd_2)
  {
    user_1_name = _user_1_name;
    user_1_blood = _user_1_blood;
    sockfd_1 = _sockfd_1;
    user_2_name = _user_2_name;
    user_2_blood = _user_2_blood;
    sockfd_2 = _sockfd_2;
    is_ready_1 = false;
    is_ready_2 = false;
  }
  void reset()
  {
    is_ready_1 = false;
    is_ready_2 = false;
  }
  int get_sockfd(string name)
  {
    if (name == user_1_name)
      return sockfd_1;
    else if (name == user_2_name)
      return sockfd_2;
    return -1;
  }
  int get_choice(string name)
  {
    if (name == user_1_name)
      return choice_1;
    else if (name == user_2_name)
      return choice_2;
    return -1;
  }
  int get_blood(string name)
  {
    if (name == user_1_name)
      return user_1_blood;
    else if (name == user_2_name)
      return user_2_blood;
    return -1;
  }
  int get_sockfd_1() { return sockfd_1; }
  int get_sockfd_2() { return sockfd_2; }
  string get_current_win()
  {
    return current_win_name;
  }
  string get_current_lose()
  {
    return current_lose_name;
  }
  bool is_win() { return user_1_blood <= 0 || user_2_blood <= 0; } // end final
  string who_win()                                                 // win final
  {
    if (user_1_blood > 0)
      return user_1_name;
    else
      return user_2_name;
  }
  string who_lose() // lose final
  {
    if (user_1_blood == 0)
      return user_1_name;
    else
      return user_2_name;
  }
  // 0 : stone |  1 : scissors |  2 : cloth
  void cut_blood()
  {
    is_round_equal = false;
    int win_1 = 0;
    if (choice_1 == 0 && choice_2 == 0)
    {
      is_round_equal = true;
      return;
    }
    else if (choice_1 == 0 && choice_2 == 1)
    {
      user_2_blood = user_2_blood >= one_ac ? user_2_blood - one_ac : 0;
      win_1 = 1;
    }
    else if (choice_1 == 0 && choice_2 == 2)
      user_1_blood = user_1_blood >= one_ac ? user_1_blood - one_ac : 0;
    else if (choice_1 == 1 && choice_2 == 0)
      user_1_blood = user_1_blood >= one_ac ? user_1_blood - one_ac : 0;
    else if (choice_1 == 1 && choice_2 == 1)
    {
      is_round_equal = true;
      return;
    }
    else if (choice_1 == 1 && choice_2 == 2)
    {
      user_2_blood = user_2_blood >= one_ac ? user_2_blood - one_ac : 0;
      win_1 = 1;
    }
    else if (choice_1 == 2 && choice_2 == 0)
    {
      user_2_blood = user_2_blood >= one_ac ? user_2_blood - one_ac : 0;
      win_1 = 1;
    }
    else if (choice_1 == 2 && choice_2 == 1)
      user_1_blood = user_1_blood >= one_ac ? user_1_blood - one_ac : 0;
    else
    {
      is_round_equal = true;
      return;
    }

    if (win_1)
    {
      current_win_name = user_1_name;
      current_lose_name = user_2_name;
    }
    else
    {
      current_win_name = user_2_name;
      current_lose_name = user_1_name;
    }
  }
  bool in_platform(string name) // fing location
  {
    if (name == user_1_name)
      return true;
    else if (name == user_2_name)
      return true;
    else
      return false;
  }
  void set_val(string name, int choice)
  {
    if (name == user_1_name)
    {
      choice_1 = choice;
      is_ready_1 = true;
    }
    else if (name == user_2_name)
    {
      choice_2 = choice;
      is_ready_2 = true;
    }
  }
  bool is_round_ready()
  {
    return is_ready_1 == true && is_ready_2 == true;
  }
  bool is_equal() { return is_round_equal; }
};

// some functions
bool user_is_exist(string youename);
void delete_client(int socket_num);
// state functions
void analyze(union Client_Buffet *cb, union Server_Buffet *sb, int socket);
void user_login(union Client_Buffet *cb, union Server_Buffet *sb, int socket);
void reback_information(union Client_Buffet *cb, union Server_Buffet *sb, int socket);
void reback_players(union Client_Buffet *cb, union Server_Buffet *sb, int socket); // reback player
void build_platform(union Client_Buffet *cb, union Server_Buffet *sb, int socket); // build platfor
void peer_responce(union Client_Buffet *cb, union Server_Buffet *sb, int socket);  //
void deal_with_game_information(union Client_Buffet *cb, union Server_Buffet *sb, int socket);