#include <iostream>
using namespace std;

#include "stdlib.h"
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <string>
#include <map>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

// Client_Buffet operation code
#define LOGIN_TEST 0x00
#define ASK_MAIN_INFORMATION 0x01
#define LOOK_FOR_RIVALS 0x02
#define CLIENT_EXIT 0x03
#define CLIENT_LOGIN 0X04
#define SEARCH_BATTLE 0x05   // used for sender
#define RESPONSE_BATTLE 0x06 // one is ready ask another used for received
#define FIGHTING_STATE 0x07  // fighting code

//  Client_Buffet operation code end
// server_Buffet key bits
#define LOGIN_STATE_SUCCESS 1 // login success
#define LOGIN_STATE_FAIL 0    // login fail
#define LOGIN_PLAYER_FULL 2   // busy
#define USER_AGREE_BATTLE 0x0 // rival agree to battle
#define USER_NOT_EXIT 0x1     // search battle
#define USER_NOT_FREE 0x2     // search battle
#define PEER_REFUSED 0x03     // search battle

#define ACCEPT_FIGHT 1 // ACCEPT the fight
#define REFUGE_FIGHT 0 // refuse

#define EQUAL 0     //[win_state]
#define WINONCE 1   // [win_state]
#define LOSEONCE 2  // [win_state]
#define FINALWIN 3  //[win_state]
#define FINALLOSE 4 //[win_state]
// server_Buffet key bits

#define NAME_SIZE 10

// some error code
//  client send buffer construction
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

// UI
void print_login_ui();
void print_main_ui();
void print_input_error();
void print_not_connection();

// some actions
int test_connection(); // return socketnum
void normal_action_read(union Server_Buffet *temp, int sockfd);
void normal_action_write(int sockfd);