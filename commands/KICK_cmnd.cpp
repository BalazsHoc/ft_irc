#include "../main.hpp"

void exec_KICK( int main_fd, std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::vector<std::string> cmnd) {
  if (cmnd.size() < 3 || cmnd.size() > 4 || (cmnd.size() == 3 && cmnd.at(2)[0] == ':') || (cmnd.size() == 4 && cmnd.at(3)[0] != ':'))
    return send_error(main_fd, clients, cli_fd, (":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters.").c_str());
  if (!check_channel(main_fd, clients, cli_fd, channels, cmnd.at(1)))
    return ;
  if (!check_client(clients, cmnd.at(2)))
    return send_error(main_fd, clients, cli_fd, (":irc.ppeter.com 441 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :User not on channel.").c_str());
  if (!check_op(main_fd, clients, cli_fd, channels, cmnd.at(1)))
    return ;
  // KICK
  if (cmnd.size() > 3)
    cmnd.at(3) = cmnd.at(3).substr(1);
  broadcast(main_fd, clients, cli_fd, channels, cmnd);
  channels[cmnd.at(1)]->drop_client(check_client(clients, cmnd.at(2)));
  clients.at(check_client(clients, cmnd.at(2)))->unset_channel(cmnd.at(1));
}
