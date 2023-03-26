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

// some resolve configurations

//  operation code
#define LOGIN_EXAMINATION 0
#define MAIN_INFORMATION 1
#define LOOK_FOR_RIVALS 2
#define CLIENT_EXIT 3
// Client_Buffet symbols
#define LOGIN_STATE_FAIL 0x00    // login fail
#define LOGIN_STATE_SUCCESS 0x01 // login success
#define LOGIN_PLAYER_FULL 0x02   //  player full
//

// client send buffer construction
union Client_Buffet
{
  struct
  {
    char operation_number;     // explare how to resolve
    char user_name[10];        // client name (use for login )
    char user_password[10];    // client password (use for login)
    unsigned char un_use[107]; //

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
};

// some functions
bool user_is_exist(string youename);
void delete_client(int socket_num);
// state functions
void analyze(union Client_Buffet *cb, union Server_Buffet *sb, int socket);
void user_login(union Client_Buffet *cb, union Server_Buffet *sb, int socket);
void reback_information(union Client_Buffet *cb, union Server_Buffet *sb, int socket);
void reback_players(union Client_Buffet *cb, union Server_Buffet *sb, int socket); // reback player