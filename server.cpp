#include "server_show.h"

////////////
int server_socket = -1;        // server's fd
int client_socket[MAX_CLIENT]; // clients' fd
////

static void recvfrom_int(int signo)
{
  cout << "server not exit!" << endl;
  close(server_socket);
  for (int i = 0; i < MAX_CLIENT; i++) // not reserve
  {
    if (client_socket[i] != 0)
    {
      close(client_socket[i]);
    }
  }
  exit(0);
}
void *handle_one_client(void *arg)
{
  // 1  [acquire according socket]
  int *socket = (int *)arg;
  int socket_num = *socket;

  // 2 [defien according message]
  union Client_Buffet cb; // send buffer
  union Server_Buffet sb; // receive buffer

  // receive message from clients
  while (recv(socket_num, cb.characters, sizeof(cb.characters), 0) > 0)
  {
    analyze(&cb, &sb, socket_num); // analyze the client message
  }
  cout << "Delete one client the sockfd is " << socket_num << endl;
  delete_client(socket_num); // delete according player and boardcast
  close(socket_num);         // close server 2 client connection

  for (int i = 0; i < MAX_CLIENT; i++) // not reserve
  {
    if (client_socket[i] == socket_num)
    {
      client_socket[i] = 0;
      break;
    }
  }
  pthread_exit(NULL); // thread disappear
}

int main()
{
  // create a thread pt
  pthread_t pt;
  //  create according socket
  server_socket = socket(PF_INET, SOCK_STREAM, 0);
  assert(server_socket != -1);

  // set socket parameters use to reserve client ip and port
  struct sockaddr_in addr;
  int addr_lenth = 0;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(SRV_PORT);

  // bind
  int bind_num = bind(server_socket, (const struct sockaddr *)(&addr), sizeof(addr));
  assert(bind_num != -1);

  // listen
  int listen_num = listen(server_socket, MAX_CLIENT_CONNECTION);
  assert(listen_num != -1);

  signal(SIGINT, recvfrom_int); // press ctrl c exit
  puts("Waiting for incoming connections...");

  // select
  fd_set fdset; // define fd set
  while (true)
  {
    for (int k = 0; k < MAX_CLIENT; k++)
      cout << client_socket[k] << " ";
    cout << endl;

    FD_ZERO(&fdset);               // clear set to zero
    FD_SET(server_socket, &fdset); // add server socket fd to fdset

    int max_fd = server_socket;          // in order to use select so find max fd
    for (int i = 0; i < MAX_CLIENT; i++) //
    {
      int current_id = client_socket[i];
      if (current_id > 0)
        FD_SET(current_id, &fdset);
      if (current_id > max_fd)
        max_fd = current_id;
    }

    // 设置等待时间
    // struct timeval timeout;
    // timeout.tv_sec = TIME_INTEVAL;
    // timeout.tv_usec = 0;

    // use select function
    int activity = select(max_fd + 1, &fdset, NULL, NULL, NULL);
    if (activity < 0 && errno == EINTR)
    {
      // select被信号中断，继续调用select
      continue;
    }
    else if (activity == 0)
    {
      cout << "Timeout reached!" << endl;
      continue;
    }
    else
    {
      // accept client socket number server fd has data to read
      if (FD_ISSET(server_socket, &fdset))
      {
        int new_socket;
        // accept a client socket
        if ((new_socket = accept(server_socket, (struct sockaddr *)&addr, (socklen_t *)&addr_lenth)) < 0)
        {
          perror("accept error");
          exit(EXIT_FAILURE);
        }
        cout << "New connection, socket fd is " << new_socket << ", ip is: " << inet_ntoa(addr.sin_addr) << ", port : " << ntohs(addr.sin_port) << endl;

        // reserve client fd
        int i = 0;
        for (; i < MAX_CLIENT; i++)
        {
          if (client_socket[i] == 0)
          {
            client_socket[i] = new_socket;
            // create thread to deal with requests
            pthread_create(&pt, NULL, handle_one_client, &new_socket);
            // detach from main process
            pthread_detach(pt);
            break;
          }
        }
        if (i == MAX_CLIENT) // too many clients
        {
          union Server_Buffet temp;
          temp.content.login_state = LOGIN_PLAYER_FULL;
          cout << "too mant clients" << endl;
          int sendflag = send(new_socket, temp.characters, sizeof(temp.characters), 0);
          assert(sendflag > 0);
        }
      }
      //
      ////////////////////////////////////////
    }
  }
  // close server socket
  close(server_socket);
  return 0;
}