#include "../main.hpp"

void exec_INVITE( int main_fd, std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::vector<std::string> cmnd) {
// INVITE: <nickname> <channel>
  if (cmnd.size() <= 2)
    return send_error(main_fd, clients, cli_fd, ":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters.");
  if (nick_available(clients, cmnd.at(1)))
    return send_error(main_fd, clients, cli_fd, ":irc.ppeter.com 401 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + " :No such nick.");
  if (!check_channel(main_fd, clients, cli_fd, channels, cmnd.at(2)))
    return ;
  if (!channels[cmnd.at(2)]->check_client(cli_fd))
    return send_error(main_fd, clients, cli_fd, ":irc.ppeter.com 442 " + clients[cli_fd]->get_nick() + space() + cmnd.at(2) + " :You are not on this channel.");
  if (channels[cmnd.at(2)]->check_client(check_client(clients, cmnd.at(1))))
    return send_error(main_fd, clients, cli_fd, (":irc.ppeter.com 443 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + space() + cmnd.at(2) + " :User is already on this channel.").c_str());
  if (channels[cmnd.at(2)]->get_invite_set() && !check_op(main_fd, clients, cli_fd, channels, cmnd.at(2)))
    return ;
  send_error(main_fd, clients, check_client(clients, cmnd.at(1)), (clients[cli_fd]->get_prefix() + space() + cmnd.at(0) + space() + cmnd.at(1) + " :" + cmnd.at(2)).c_str());
  send_error(main_fd, clients, cli_fd, ":irc.ppeter.com 341 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + space() + cmnd.at(2));
  clients[check_client(clients, cmnd.at(1))]->set_invite(cmnd.at(2));
}


