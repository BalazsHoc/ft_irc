#include "../main.hpp"

void exec_JOIN(int main_fd, std::map<int, Client *> &clients, int cli_fd, std::vector<std::string> cmnd, std::map<std::string, Channel *> &channels) {
  if (clients[cli_fd]->get_channel_count() == 10)
    return send_error(main_fd, clients, cli_fd, ":irc.ppeter.com 405 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + " :You have joined too many channels.");
  if (cmnd.at(1).empty() || cmnd.at(1)[0] != '#' || !valid_chars(cmnd.at(1)))
    return send_error(main_fd, clients, cli_fd,":irc.ppeter 476 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + " :Bad Channel Mask.");
  try {
    if (channels.at(cmnd.at(1))->get_pass_set()) { // NOTE: this throws if channel does not exist yet. we leave it and do not use check_channel as JOIN is ONLY cmnd which allows creating one when channel does not exist yet.
      if (cmnd.size() <= 2 || channels[cmnd.at(1)]->get_pass() != cmnd.at(2))
        return send_error(main_fd, clients, cli_fd,":irc.ppeter 475 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + " :Bad channel key.");
    }
    if (channels[cmnd.at(1)]->check_client(cli_fd))
      return ; // DO NOTHING WHEN ALREADY ON CHANNEL
    if (channels[cmnd.at(1)]->get_invite_set() && !clients[cli_fd]->check_invited(cmnd.at(1)))
      return send_error(main_fd, clients, cli_fd, ":irc:pperter.com 473 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + " :Cannot join channel (+i).");
    if (channels[cmnd.at(1)]->get_user_count() >= channels[cmnd.at(1)]->get_limit())
      return send_error(main_fd, clients, cli_fd, ":irc:pperter.com 471 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + " :Channel is full.");
    // JOINS THE CHANNEL
    clients[cli_fd]->set_channel(cmnd.at(1));
    channels[cmnd.at(1)]->set_client(cli_fd, clients[cli_fd]->get_nick());
  } catch (const std::out_of_range &e) {
    // CREATES & JOINS THE CHANNEL AS OP
    Channel *new_one = new Channel();
    channels[cmnd.at(1)] = new_one;
    new_one->set_client(cli_fd, clients[cli_fd]->get_nick());
    clients[cli_fd]->set_channel(cmnd.at(1));
    new_one->set_name(cmnd.at(1));
    new_one->set_op(cli_fd, clients[cli_fd]->get_nick(), 1);
  }
  broadcast(main_fd, clients, cli_fd, channels, cmnd);
  if (channels[cmnd.at(1)]->get_topic() != "") {
    send_error(main_fd, clients, cli_fd,":irc.ppeter 332 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + space() + channels[cmnd.at(1)]->get_topic());
  } else
    send_error(main_fd, clients, cli_fd,":irc.ppeter 331 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + " :No topic is set");
  send_error(main_fd, clients, cli_fd, send_annoying_error(":irc.ppeter 353 " + clients[cli_fd]->get_nick() + space() + "=" + space() + cmnd.at(1) + space() + ":", names_list(channels[cmnd.at(1)]), ""));
  send_error(main_fd, clients, cli_fd,":irc.ppeter 366 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + " :End of /NAMES list");
}


