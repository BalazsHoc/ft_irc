#include "../main.hpp"

int check_channel(int main_fd, std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::string channel) {
  try {
    if (channels.at(channel)) {
      if (!channels.at(channel)->check_client(cli_fd))
        return send_error(main_fd, clients, cli_fd, ":irc.ppeter.com 442 " + clients[cli_fd]->get_nick() + space() + channel + " :You are not on this channel."), 0;
      return 1;
    }
  } catch (const std::out_of_range &e) {
    return send_error(main_fd, clients, cli_fd,":irc.ppeter 403 " + clients[cli_fd]->get_nick() + space() + channel + " :No such channel."), 0;
  }
  return 0;
}

int check_op(int main_fd, std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::string channel) {
  if (!check_channel(main_fd, clients, cli_fd, channels, channel))
      return 0;
  if (!channels.at(channel)->check_op(cli_fd))
    return send_error(main_fd, clients, cli_fd,":irc.ppeter 482 " + clients[cli_fd]->get_nick() + space() + channel + " :Channel operator privileges needed."), 0;
  return 1;
}

int check_client( std::map<int, Client *> &clients, std::string nick ) {
  try {
    for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); it++) {
      if (it->second->get_nick() == nick) {
        return it->first;
      }
    }
  } catch (std::exception &e) {
    // for debugging
  }
  return 0;
}

int nick_available(std::map<int, Client *> &clients, std::string nick) {
  for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); ++it) {
    if (it->second->get_nick() == nick)
      return 0;
  }
  return 1;
}
