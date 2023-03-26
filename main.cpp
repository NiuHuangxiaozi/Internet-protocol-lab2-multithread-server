#include "show.h"

// some configurations
int sockfd = -1; // our socket number

// client party

int main(int argc, char *argv[])
{

  // 创建socket
  try // test whether connection is right
  {
    sockfd = test_connection();
  }
  catch (int error_code)
  {
    exit(-1); // exit
  }

  while (true)
  {
    try
    {
      normal_action(sockfd);
    }
    catch (int flag)
    {
      if (flag == 1)
        printf("The server is dump\n");
      break;
    }
  }
  close(sockfd);
  return 0;
}
