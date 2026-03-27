#include "../main.hpp"

void broadcast( int main_fd, std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::vector<std::string> cmnd) {
  std::string channel = cmnd.at(1);
  std::map<int, std::string> channel_clients = channels[channel]->get_clients();
  for (std::map<int, std::string>::iterator it = channel_clients.begin(); it != channel_clients.end(); it++) {
    if (cmnd.size() == 4)
      send_error(main_fd, clients, it->first, clients[cli_fd]->get_prefix() + space() + cmnd.at(0) + space() + cmnd.at(1) + space() + cmnd.at(2) + space() + cmnd.at(3));
    else if (cmnd.size() == 3) //                                                                         we need this space in case of mode +i // but we also need the colon in case of TOPIC
      send_error(main_fd, clients, it->first, clients[cli_fd]->get_prefix() + space() + cmnd.at(0) + space() + cmnd.at(1) + space() + cmnd.at(2));
    else
      send_error(main_fd, clients, it->first, clients[cli_fd]->get_prefix() + space() + cmnd.at(0) + space() + cmnd.at(1));
  }
}

std::vector<std::string> names_list( Channel *channel ) {
  std::vector<std::string> list;
  std::map<int, std::string> clients = channel->get_clients();
  std::map<int, std::string> ops = channel->get_ops();

  for ( std::map<int, std::string>::iterator it = clients.begin(); it != clients.end(); it++ ) {
    std::map<int, std::string>::iterator sec = find(ops.begin(), ops.end(), *it);
    if (sec != ops.end())
      list.push_back("@" + it->second + space());
  }
  for ( std::map<int, std::string>::iterator it = clients.begin(); it != clients.end(); it++ ) {
    std::map<int, std::string>::iterator sec = find(ops.begin(), ops.end(), *it);
    if (sec == ops.end())
      list.push_back(it->second + space());
  }
  return list;
}

std::string send_from_cmnd( std::string msg_part_1, std::vector<std::string> middle_msgs, std::string msg_part_2 ) {
  std::vector<std::string>::iterator next_one;
  msg_part_1.append(space());
  for (std::vector<std::string>::iterator it = middle_msgs.begin(); it != middle_msgs.end(); it++ ) {
    next_one = it + 1;
    if (it != middle_msgs.begin() && next_one != middle_msgs.end())
      msg_part_1.append(*it + space());
  }
  if (!msg_part_2.empty())
    msg_part_1.append(msg_part_2);
  return msg_part_1;
}

std::string send_annoying_error( std::string msg_part_1, std::vector<std::string> middle_msgs, std::string msg_part_2 ) {
  for (std::vector<std::string>::iterator it = middle_msgs.begin(); it != middle_msgs.end(); it++ ) {
    msg_part_1.append(*it);
  }
  if (!msg_part_2.empty())
    msg_part_1.append(msg_part_2);
  return msg_part_1;
}
