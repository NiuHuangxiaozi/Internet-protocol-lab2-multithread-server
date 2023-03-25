#include "show.h"

// some configurations
int sockfd = -1;        // our socket number
union Client_Buffet cb; // send buffer
union Server_Buffet sb; // receive buffer

enum user_state
{
  LOGIN_IN = 0,
  BASE_UI = 1
};

void normal_action()
{
  int send_flag;
  int recv_flag;
  int state = 0; // user initial state
  while (true)
  {
    switch (state)
    {
    case LOGIN_IN: // login stage
      cout << "user name" << endl;
      cin >> cb.content.user_name;
      cout << "user password " << endl;
      cin >> cb.content.user_password;

      send_flag = send(sockfd, cb.characters, sizeof(cb.characters), 0);
      if (send_flag < 0)
        return;
      recv_flag = recv(sockfd, sb.characters, sizeof(sb.characters), 0);
      if (recv_flag < 0)
        return;
      if (int(sb.content.login_state) == LOGIN_STATE_SUCCESS)
        cout << "Login in successfully" << endl;
      else
        cout << "Login in failly" << endl;
      break;
    };
  }
}

// client party

int main()
{

  try
  {
    sockfd = test_connection(); // test whether connection is right
  }
  catch (int error_code)
  {
    exit(-1); // exit
  }
  normal_action();
  close(sockfd);
  return 0;
}
