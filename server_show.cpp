#include "server_show.h"

//
map<string, int> name2index; // name 2 index
vector<Client> clients;      // current  clients
vector<Platform> platforms;  // platforms

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
  // delete client
  string name;
  int state;
  int map_adjust_loc = int(clients.size());
  for (int j = 0; j < int(clients.size()); j++)
  {
    if (clients[j].get_sockfd() == socket_num)
    {
      name = clients[j].get_name();
      state = clients[j].get_state();
      clients.erase(clients.begin() + j);
      map_adjust_loc = j;
    }
    if (j >= map_adjust_loc)
      name2index[clients[j].get_name()]--;
  }

  // tell other clients
  for (int k = 0; k < int(clients.size()); k++)
  {
    union Server_Buffet sb;
    sb.content.operation_number = CLIENT_EXIT;
    strcpy(sb.content.user_name, name.c_str());
    sb.content.state = state;
    int send_flag = send(clients[k].get_sockfd(), sb.characters, sizeof(sb.characters), 0);
    assert(send_flag != -1);
  }

  // delete platform
  for (int i = 0; i < int(platforms.size()); i++)
  {
    if (platforms[i].in_platform(name))
    {
      // tell counterpart turn to base_ui
      // ignore the operation number
      union Server_Buffet sb;
      sb.content.operation_number = CLIENT_EXIT;

      // memset(sb.content.peer_name, 0, sizeof(sb.content.peer_name));
      // strcpy(sb.content.peer_name, name.c_str());
      // sb.content.peer_name[sizeof(sb.content.peer_name) - 1] = '\0';

      int send_flag = send(clients[name2index[platforms[i].get_counterpart_name(name)]].get_sockfd(),
                           sb.characters, sizeof(sb.characters), 0);
      assert(send_flag != -1);
      // set client to base ui
      clients[name2index[platforms[i].get_counterpart_name(name)]].set_online();
      //  delete platform
      platforms.erase(platforms.begin() + i);
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
    break;
  case LOOK_FOR_RIVALS:
    reback_players(cb, sb, socket); // reback players
    break;
  case SEARCH_BATTLE:
    build_platform(cb, sb, socket); // build platform
    break;
  case RESPONSE_BATTLE:            // another peer responce (agree or refuse)
    peer_responce(cb, sb, socket); //
    break;
  case FIGHTING_STATE:                          // ongoing game
    deal_with_game_information(cb, sb, socket); // dealwith game information
    break;
  default:
    break;
  }
}

// follows are  message dealwith
// login test
void user_login(union Client_Buffet *cb, union Server_Buffet *sb, int socket)
{
  string uer_name = cb->content.user_name;
  bool login_succ = false;
  if (user_is_exist(uer_name)) // fail
    sb->content.login_state = LOGIN_STATE_FAIL;
  else // successful
  {
    sb->content.login_state = LOGIN_STATE_SUCCESS;
    memset(sb->content.user_name, 0, NAME_SIZE);
    strcpy(sb->content.user_name, uer_name.c_str());
    sb->content.user_name[NAME_SIZE - 1] = '\0';
    Client cli(uer_name, socket, "123456"); // create
    cli.set_online();
    clients.push_back(cli);
    name2index[uer_name] = clients.size() - 1;
    login_succ = true;
  }
  int send_flag = send(socket, sb->characters, sizeof(sb->characters), 0);
  assert(send_flag != -1);
  if (login_succ)
  {
    // boardcast to other members boardcast the last one
    for (int j = 0; j < int(clients.size()) - 1; j++)
    {
      union Server_Buffet sb;
      sb.content.operation_number = CLIENT_LOGIN;
      strcpy(sb.content.user_name, uer_name.c_str());
      sb.content.state = 0;
      int send_flag = send(clients[j].get_sockfd(), sb.characters, sizeof(sb.characters), 0);
      assert(send_flag != -1);
    }
  }
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
void build_platform(union Client_Buffet *cb, union Server_Buffet *sb, int socket)
{
  string rival_name = cb->content.rival_name;
  string user_name = cb->content.user_name;
  clients[name2index[user_name]].set_ready();
  int user_location = -1;
  sb->content.operation_number = SEARCH_BATTLE;
  if (name2index.find(rival_name) != name2index.end())
    user_location = name2index[rival_name];
  else // user not found
  {
    sb->content.find_rival_error_code = USER_NOT_EXIT;
    int send_flag = send(socket, sb->characters, sizeof(sb->characters), 0);
    assert(send_flag != -1);
    clients[name2index[user_name]].set_online();
    return;
  }

  // user state not well
  if (clients[user_location].get_state() != 1)
  {
    sb->content.find_rival_error_code = USER_NOT_FREE;
    int send_flag = send(socket, sb->characters, sizeof(sb->characters), 0);
    assert(send_flag != -1);
    clients[name2index[user_name]].set_online();
  }
  clients[name2index[rival_name]].set_ready();
  // build playform
  Platform pf(clients[name2index[user_name]].get_name(),
              clients[name2index[user_name]].get_blood(),
              clients[name2index[user_name]].get_sockfd(),
              clients[name2index[rival_name]].get_name(),
              clients[name2index[rival_name]].get_blood(),
              clients[name2index[rival_name]].get_sockfd());
  platforms.push_back(pf); // use in case one client is dump
  //  ask another
  // set another peer to ready
  // send message to another peer
  int riral_sockfd = clients[user_location].get_sockfd();
  sb->content.operation_number = RESPONSE_BATTLE;
  //
  memset(sb->content.peer_name, 0, sizeof(sb->content.peer_name));
  strcpy(sb->content.peer_name, user_name.c_str());
  sb->content.peer_name[sizeof(sb->content.peer_name) - 1] = '\0';
  memset(sb->content.user_name, 0, sizeof(sb->content.user_name));
  strcpy(sb->content.user_name, rival_name.c_str());
  sb->content.user_name[sizeof(sb->content.user_name) - 1] = '\0';
  //
  int send_flag = send(riral_sockfd, sb->characters, sizeof(sb->characters), 0);
  assert(send_flag != -1);
  cout << "send to " << clients[user_location].get_name() << endl;
}

void peer_responce(union Client_Buffet *cb, union Server_Buffet *sb, int socket)
{
  if (int(cb->content.whether_accept) == REFUGE_FIGHT) // refuse
  {
    // set client to free
    cout << "refuse" << endl;
    clients[name2index[cb->content.user_name]].set_online();
    clients[name2index[cb->content.rival_name]].set_online();
    // send refuse message to original sender
    sb->content.operation_number = SEARCH_BATTLE;
    sb->content.find_rival_error_code = PEER_REFUSED;
    int send_flag =
        send(clients[name2index[cb->content.rival_name]].get_sockfd(), sb->characters, sizeof(sb->characters), 0);
    assert(send_flag != -1);
  }
  else if (int(cb->content.whether_accept) == ACCEPT_FIGHT) // agree
  {
    cout << "agree" << endl;
    // all in fighting state
    clients[name2index[cb->content.user_name]].set_combat();
    clients[name2index[cb->content.rival_name]].set_combat();
    sb->content.operation_number = SEARCH_BATTLE;
    sb->content.find_rival_error_code = USER_AGREE_BATTLE;
    int send_flag =
        send(clients[name2index[cb->content.rival_name]].get_sockfd(), sb->characters, sizeof(sb->characters), 0);
    assert(send_flag != -1);
  }
}

void deal_with_game_information(union Client_Buffet *cb, union Server_Buffet *sb, int socket)
{
  // check each other is online

  // dealwith
  string player_name = cb->content.user_name;
  int choice = int(cb->content.choice);
  int platform_loc = -1;
  for (int i = 0; i < int(platforms.size()); i++)
  {
    if (platforms[i].in_platform(player_name))
    {
      platform_loc = i;
      break;
    }
  }

  // set self
  platforms[platform_loc].set_val(player_name, choice);

  // judge whether each other is ready
  if (platforms[platform_loc].is_round_ready())
  {
    platforms[platform_loc].cut_blood();
    // equal
    if (platforms[platform_loc].is_win()) // judge whether end final
    {
      sb->content.operation_number = FIGHTING_STATE;
      string final_win_name = platforms[platform_loc].who_win();
      string final_lose_name = platforms[platform_loc].who_lose();
      // send winer
      sb->content.win_state = FINALWIN;
      memset(sb->content.user_name, 0, sizeof(sb->content.user_name));
      strcpy(sb->content.user_name, final_win_name.c_str());
      sb->content.user_name[sizeof(sb->content.user_name) - 1] = '\0';
      //
      memset(sb->content.peer_name, 0, sizeof(sb->content.peer_name));
      strcpy(sb->content.peer_name, final_lose_name.c_str());
      sb->content.peer_name[sizeof(sb->content.peer_name) - 1] = '\0';

      int send_flag1 = // send two
          send(platforms[platform_loc].get_sockfd(final_win_name), sb->characters, sizeof(sb->characters), 0);
      assert(send_flag1 != -1);

      // send loser
      sb->content.win_state = FINALLOSE;
      memset(sb->content.user_name, 0, sizeof(sb->content.user_name));
      strcpy(sb->content.user_name, final_lose_name.c_str());
      sb->content.user_name[sizeof(sb->content.user_name) - 1] = '\0';
      //
      memset(sb->content.peer_name, 0, sizeof(sb->content.peer_name));
      strcpy(sb->content.peer_name, final_win_name.c_str());
      sb->content.peer_name[sizeof(sb->content.peer_name) - 1] = '\0';

      int send_flag2 = // send two
          send(platforms[platform_loc].get_sockfd(final_lose_name), sb->characters, sizeof(sb->characters), 0);
      assert(send_flag2 != -1);

      // delete platform and set satte to free
      clients[name2index[final_win_name]].set_online();
      clients[name2index[final_lose_name]].set_online();
      platforms.erase(platforms.begin() + platform_loc);
      return;
    }
    else // one win or equal
    {
      if (platforms[platform_loc].is_equal()) // equal
      {
        sb->content.operation_number = FIGHTING_STATE;
        sb->content.win_state = EQUAL;
        int send_flag1 = // send one
            send(platforms[platform_loc].get_sockfd_1(), sb->characters, sizeof(sb->characters), 0);
        assert(send_flag1 != -1);
        int send_flag2 = // send two
            send(platforms[platform_loc].get_sockfd_2(), sb->characters, sizeof(sb->characters), 0);
        assert(send_flag2 != -1);
      }
      else // one  win
      {
        string win_name = platforms[platform_loc].get_current_win();
        string lose_name = platforms[platform_loc].get_current_lose();
        sb->content.operation_number = FIGHTING_STATE;
        // send win message
        // 1 username
        sb->content.win_state = WINONCE;
        memset(sb->content.user_name, 0, sizeof(sb->content.user_name));
        strcpy(sb->content.user_name, win_name.c_str());
        sb->content.user_name[sizeof(sb->content.user_name) - 1] = '\0';
        // 2 rivalname
        memset(sb->content.peer_name, 0, sizeof(sb->content.peer_name));
        strcpy(sb->content.peer_name, lose_name.c_str());
        sb->content.peer_name[sizeof(sb->content.peer_name) - 1] = '\0';

        sb->content.blood = platforms[platform_loc].get_blood(lose_name);
        sb->content.user_name_choice = platforms[platform_loc].get_choice(win_name);
        sb->content.peer_name_choice = platforms[platform_loc].get_choice(lose_name);

        int send_flag1 = // send two
            send(platforms[platform_loc].get_sockfd(win_name), sb->characters, sizeof(sb->characters), 0);
        assert(send_flag1 != -1);
        // send lose message
        sb->content.win_state = LOSEONCE;

        memset(sb->content.user_name, 0, sizeof(sb->content.user_name));
        strcpy(sb->content.user_name, lose_name.c_str());
        sb->content.user_name[sizeof(sb->content.user_name) - 1] = '\0';
        // 2 rivalname
        memset(sb->content.peer_name, 0, sizeof(sb->content.peer_name));
        strcpy(sb->content.peer_name, win_name.c_str());
        sb->content.peer_name[sizeof(sb->content.peer_name) - 1] = '\0';

        sb->content.blood = platforms[platform_loc].get_blood(win_name);
        sb->content.user_name_choice = platforms[platform_loc].get_choice(lose_name);
        sb->content.peer_name_choice = platforms[platform_loc].get_choice(win_name);

        int send_flag2 = // send two
            send(platforms[platform_loc].get_sockfd(lose_name), sb->characters, sizeof(sb->characters), 0);
        assert(send_flag2 != -1);
      }

      platforms[platform_loc].reset(); // reset ready for next round
    }
  }
}