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
  string name;
  int state;
  for (int j = 0; j < int(clients.size()); j++)
  {
    if (clients[j].get_sockfd() == socket_num)
    {
      name = clients[j].get_name();
      state = clients[j].get_state();
      clients.erase(clients.begin() + j);
      break;
    }
  }
  for (int k = 0; k < int(clients.size()); k++)
  {
    union Server_Buffet sb;
    sb.content.operation_number = CLIENT_EXIT;
    strcpy(sb.content.user_name, name.c_str());
    sb.content.state = state;
    int send_flag = send(clients[k].get_sockfd(), sb.characters, sizeof(sb.characters), 0);
    assert(send_flag != -1);
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
    break;
  case LOOK_FOR_RIVALS:
    reback_players(cb, sb, socket); // reback players
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
  cout << "This is reback informaintion" << endl;
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
      break;
    }
  }
}

void reback_players(union Client_Buffet *cb, union Server_Buffet *sb, int socket) // reback players
{
  cout << "This is reback players" << endl;
  sb->content.operation_number = LOOK_FOR_RIVALS;
  sb->content.player_number = clients.size();
  int namesize = 10;
  int statesize = 1;
  for (int i = 0; i < int(clients.size()); i++)
  {
    memcpy(sb->content.members + i * (namesize + statesize), clients[i].get_name().c_str(), namesize);
    sb->content.members[namesize * (i + 1)] = clients[i].get_state();
  }
  int send_flag = send(socket, sb->characters, sizeof(sb->characters), 0);
  assert(send_flag != -1);
}