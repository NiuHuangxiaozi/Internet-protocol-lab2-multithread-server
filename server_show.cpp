#include "server_show.h"

//
map<string, int> name2index; // name 2 index
vector<Client> clients;      // current  clients

bool user_is_exist(string yourname)
{
  for (int i = 0; i < int(clients.size()); i++)
  {
    if (clients[i].test_identity(yourname))
    {
      return true;
    }
  }
  return false;
}

void delete_client(int socket_num)
{
  for (int j = 0; j < int(clients.size()); j++)
  {
    if (clients[j].get_sockfd() == socket_num)
    {
      clients.erase(clients.begin() + j);
      break;
    }
  }
  return;
}

// state function
void analyze(union Client_Buffet *cb, union Server_Buffet *sb, int socket)
{
  int opera = cb->content.operation_number;
  switch (opera)
  {
  case LOGIN_EXAMINATION:
    user_login(cb, sb, socket); // login test
    break;
  case MAIN_INFORMATION:
    reback_information(cb, sb, socket); // reback information
  default:
    break;
  }
}

// follows are  message dealwith
// login test
void user_login(union Client_Buffet *cb, union Server_Buffet *sb, int socket)
{
  string uer_name = cb->content.user_name;
  if (user_is_exist(uer_name)) // fail
    sb->content.login_state = LOGIN_STATE_FAIL;
  else // successful
  {
    sb->content.login_state = LOGIN_STATE_SUCCESS;
    Client cli(uer_name, socket, "123456"); // create
    clients.push_back(cli);
    name2index[uer_name] = clients.size() - 1;
  }
  int send_flag = send(socket, sb->characters, sizeof(sb->characters), 0);
  assert(send_flag != -1);
}

void reback_information(union Client_Buffet *cb, union Server_Buffet *sb, int socket)
{
  string uer_name = cb->content.user_name;
  for (int index = 0; index < int(clients.size()); index++)
  {
    if (clients[index].get_name() == uer_name)
    {
      sb->content.operation_number = cb->content.operation_number;
      sb->content.blood = clients[index].get_blood();
      sb->content.state = clients[index].get_state();
      strcpy(sb->content.user_name, clients[index].get_name().c_str());
      strcpy(sb->content.user_password, clients[index].get_password().c_str());
      int send_flag = send(socket, sb->characters, sizeof(sb->characters), 0);
      assert(send_flag != -1);
    }
  }
}
