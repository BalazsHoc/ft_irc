#include "../main.hpp"

void exec_MSG( int main_fd, std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::vector<std::string> cmnd) {
  if (cmnd.size() == 2 && cmnd.at(1)[0] == ':')
    return send_error(main_fd, clients, cli_fd, (":irc.ppeter.com 411 " + clients[cli_fd]->get_nick() + " :No recipient given (PRIVMSG)"));
  if (cmnd.size() == 2)
    return send_error(main_fd, clients, cli_fd, (":irc.ppeter.com 412 " + clients[cli_fd]->get_nick() + " :No text to send."));
  if (cmnd.at(1)[0] == '#') {
    if (!check_channel(main_fd, clients, cli_fd, channels, cmnd.at(1)))
      return ;
    // SENDING TO CHANNEL..
    broadcast(main_fd, clients, cli_fd, channels, cmnd);
  } else {
    if (!check_client(clients, cmnd.at(1)))
      return send_error(main_fd, clients, cli_fd, (":irc.ppeter.com 401 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + " :No such nick."));
    // SENDING TO CLIENT..
    // send_error(check_client(clients, cmnd.at(1)), ":" + clients[cli_fd]->get_nick() + "!" + clients[cli_fd]->get_user() + "@" + clients[cli_fd]->get_host() + space() + cmnd.at(0) + space() + cmnd.at(2), 0);
    send_error(main_fd, clients, check_client(clients, cmnd.at(1)), clients[cli_fd]->get_prefix() + space() + cmnd.at(0) + space() + cmnd.at(1) + space() + cmnd.at(2));
  }
}

