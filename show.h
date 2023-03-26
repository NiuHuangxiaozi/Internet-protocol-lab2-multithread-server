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

//  Client_Buffet operation code end
// server_Buffet key bits
#define LOGIN_STATE_SUCCESS 1 // login success
#define LOGIN_STATE_FAIL 0    // login fail
#define LOGIN_PLAYER_FULL 2   // busy
// server_Buffet key bits

// client send buffer construction
union Client_Buffet
{
  struct
  {
    char operation_number;  // explare how to resolve
    char user_name[10];     // client name (use for login )
    char user_password[10]; // client password (use for login)
    char un_use[107];       //

  } content;
  char characters[128];
};

union Server_Buffet
{
  struct
  {
    char operation_number;  // explare how to resolve
    char login_state;       // whether login successful
    char user_name[10];     // client name (use for login )
    char user_password[10]; // client password (use for login)
    char blood;             // client blood
    char state;             // client state
    char player_number;     // player's number (max 3)
    char members[11 * 3];   // player's (name ,state)
    char no_use[70];        // leave to use
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