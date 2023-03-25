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
using namespace std;

// some configurations
#define SRV_PORT 4321           // server's port
#define MAX_CLIENT_CONNECTION 3 // max client connection contemporary
#define MAX_CLIENT 10           // max client to serve
#define TIME_INTEVAL 20         // 设置等待时间,select function wait inteval

// sone resolve configurations
//  1[Client_Buffet operation code
#define LOGIN_TEST 0x01

//  Client_Buffet operation code end
//  2[Server_Buffet operation code

//  end Server_Buffet operation code

// analyze_state
#define LOGIN_EXAMINATION 0

// client send buffer construction
union Client_Buffet
{
  struct
  {
    unsigned char operation_number[2]; // explare how to resolve
    unsigned char user_name[10];       // client name
    unsigned char user_password[10];   // client password
    unsigned char un_use[106];         //

  } content;
  char characters[128];
};

// Client_Buffet operation code
#define LOGIN_STATE_SUCCESS 0x01 // login success
#define LOGIN_STATE_FAIL 0x00    // login fail

//  Client_Buffet operation code end

union Server_Buffet
{
  struct
  {
    unsigned char operation_number[2]; // explare how to resolve
    unsigned char login_state;         // whether login successful
    char no_use[125];                  // leave to use
  } content;
  char characters[128];
};

// some functions
void analyze(union Client_Buffet *cb, union Server_Buffet *sb);