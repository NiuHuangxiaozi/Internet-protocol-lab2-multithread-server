#include "show.h"

// some configurations
int sockfd = -1; // our socket number
bool is_reset = false;
int count = 0;
pthread_mutex_t count_mutex;
pthread_cond_t count_threshold_cv;

void *Send_Message(void *arg)
{
  while (true)
  {
    try
    {
      normal_action_write(sockfd);
    }
    catch (int symbol)
    {
      if (symbol == 1) // client exit
      {
        is_reset = true;
        break;
      }
      else if (symbol == 2) // clear intend to ignore pthread_wait
      {
        continue;
      }
    }
    catch (string s)
    {
    }
    if (is_reset)
      break;
    pthread_cond_wait(&count_threshold_cv, &count_mutex);
  }
  pthread_exit(NULL);
}
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
  pthread_attr_t attr;
  /* Initialize mutex and condition variable objects */
  pthread_mutex_init(&count_mutex, NULL);
  pthread_cond_init(&count_threshold_cv, NULL);
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  // create thread to send message
  pthread_t thread;
  pthread_create(&thread, &attr, Send_Message, NULL);

  fd_set rfds;
  while (true)
  {
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds); // server fd

    int end = sockfd + 1;
    select(end, &rfds, NULL, NULL, NULL);
    if (FD_ISSET(sockfd, &rfds))
    {
      union Server_Buffet sb;
      bool recv_flag = recv(sockfd, sb.characters, sizeof(sb.characters), 0);
      if (recv_flag <= 0)
      {
        printf("recv error or server closed\n");
        is_reset = true;
        break;
      }
      try
      {
        normal_action_read(&sb, sockfd);
      }
      catch (int error)
      {
        is_reset = true;
      }
      catch (string s)
      {
        continue;
      }
    }
    if (is_reset) // back code
      break;
    pthread_cond_signal(&count_threshold_cv);
  }
  pthread_join(thread, NULL);
  close(sockfd);

  /* Clean up and exit */
  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&count_mutex);
  pthread_cond_destroy(&count_threshold_cv);
  return 0;
}
