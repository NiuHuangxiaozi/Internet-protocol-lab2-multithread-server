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
void analyze(union Client_Buffet *cb, union Server_Buffet *sb, int socket)
{
  int operation_higher_bit = cb->content.operation_number[0];
  int operation_lower_bit = cb->content.operation_number[1];
  switch (operation_higher_bit)
  {
  case 0:
    switch (operation_lower_bit)
    {
    case LOGIN_EXAMINATION:
      user_login(cb, sb, socket); // login test
      break;
    }
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
    Client cli(uer_name, "123456"); // create
    clients.push_back(cli);
    name2index[uer_name] = clients.size() - 1;
  }
  int send_flag = send(socket, sb->characters, sizeof(sb->characters), 0);
  assert(send_flag != -1);
}
