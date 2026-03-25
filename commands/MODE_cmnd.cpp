#include "../main.hpp"

std::string current_modes(Channel *channel) {
  if (!channel)
    return "+";

  std::string modes = "+";
  std::ostringstream params;

  if (channel->get_invite_set())
    modes += 'i';
  if (channel->get_topic_set())
    modes += 't';
  if (channel->get_pass_set()) {
    modes += 'k';
    params << " " << channel->get_pass();
  }
  if (channel->get_limit() != 200) {
    modes += 'l';
    params << " " << channel->get_limit();
  }
  if (modes == "+")
    return "+";
  return modes + params.str();
}


void exec_MODE( int main_fd, std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::vector<std::string> cmnd) {
  if (!check_channel(main_fd, clients, cli_fd, channels, cmnd.at(1)))
    return ;
  // TODO: CHECK ORDER WITH EACH INDIVIDUAL 'Not enought parameters.'
  if (cmnd.size() == 2)
      return send_error(main_fd, clients, cli_fd, ":irc.ppeter.com 324 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + space() + current_modes(channels.at(cmnd.at(1))));
  if (!check_op(main_fd, clients, cli_fd, channels, cmnd.at(1)))
    return ;
  else if (cmnd.at(2) == "+i" || cmnd.at(2) == "-i") {
    if (cmnd.size() != 3)
      return send_error(main_fd, clients, cli_fd, (":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters."));
    if (cmnd.at(2) == "+i")
      channels[cmnd.at(1)]->set_invite_set(true);
    else
      channels[cmnd.at(1)]->set_invite_set(false);
  } else if ( cmnd.at(2) == "+t" || cmnd.at(2) == "-t") {
    if (cmnd.size() != 3)
      return send_error(main_fd, clients, cli_fd, (":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters.").c_str());
    channels[cmnd.at(1)]->set_topic_set(cmnd.at(2) == "+t");
  } else if ( cmnd.at(2) == "+k" || cmnd.at(2) == "-k") {
    if (cmnd.at(2) == "+k" && cmnd.size() != 4)
      return send_error(main_fd, clients, cli_fd, (":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters.").c_str());
    else if (cmnd.at(2) == "-k" && cmnd.size() != 3 && printf("FUCK THIS SHIT\n"))
      return send_error(main_fd, clients, cli_fd, (":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters.").c_str());
    if (cmnd.at(2) == "+k") {
      channels[cmnd.at(1)]->set_pass(cmnd.at(3));
      cmnd.at(3) = "*";
    } else
      channels[cmnd.at(1)]->unset_pass();
  } else if ( cmnd.at(2) == "+o" || cmnd.at(2) == "-o") {
    if (cmnd.size() != 4)
      return send_error(main_fd, clients, cli_fd, (":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters.").c_str());
    if (!check_client(clients, cmnd.at(3)))
      return send_error(main_fd, clients, cli_fd, ":irc.ppeter.com 441 " + clients[cli_fd]->get_nick() + space() + cmnd.at(2) + ":User is not on this channel.");
    if (!channels[cmnd.at(1)]->check_client(check_client(clients, cmnd.at(3))))
      return ;
    // SET OP
    if (cmnd.at(2) == "+o") // TODO: do we need to check_client()???
      channels[cmnd.at(1)]->set_op(check_client(clients, cmnd.at(3)), cmnd.at(3), 1);
    else // UNSET OP
      channels[cmnd.at(1)]->set_op(check_client(clients, cmnd.at(3)), cmnd.at(3), 0);
  } else if ( cmnd.at(2) == "+l" || cmnd.at(2) == "-l") {
    if (cmnd.at(2) == "+l" && cmnd.size() != 4)
      return send_error(main_fd, clients, cli_fd, (":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters.").c_str());
    else if (cmnd.at(2) == "-l" && cmnd.size() != 3)
      return send_error(main_fd, clients, cli_fd, (":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters.").c_str());
    if (cmnd.at(2) == "+l" && !channels[cmnd.at(1)]->set_limit(cmnd.at(3))) // SET LIMIT
      return send_error(main_fd, clients, cli_fd, (":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters.").c_str());
    else if (cmnd.at(2) == "-l")
      channels[cmnd.at(1)]->unset_limit();
  } else
    return send_error(main_fd, clients, cli_fd, (":irc.ppeter.com 472 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Unknown mode.").c_str());
  broadcast(main_fd, clients, cli_fd, channels, cmnd);
}
