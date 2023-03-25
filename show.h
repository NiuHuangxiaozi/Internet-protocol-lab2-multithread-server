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

// Client_Buffet operation code
#define LOGIN_TEST 0x0001

//  Client_Buffet operation code end

// client send buffer construction
union Client_Buffet
{
  struct
  {
    unsigned char operation_number[2]; // explare how to resolve
    char user_name[10];                // client name
    char user_password[10];            // client password
    unsigned char un_use[106];         //

  } content;
  char characters[128];
};

// Client_Buffet operation code
#define LOGIN_STATE_SUCCESS 1 // login success
#define LOGIN_STATE_FAIL 0    // login fail

//  Client_Buffet operation code end

union Server_Buffet
{
  struct
  {
    unsigned char operation_number[2]; // explare how to resolve
    char login_state;                  // whether login successful
    char no_use[125];                  // leave to use
  } content;
  char characters[128];
};

// UI
void print_main_ui();
void print_not_exist_city(string s);
void print_whether_select();
void print_input_error();

// some actions
void reserve_weather();
int test_connection(); // return socketnum
int send_city_name(char *city_name, int sockfd);
int display_city_whether(string operation, int sockfd); // display city's whether
