#include "../main.hpp"

void exec_TOPIC( int main_fd, std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::vector<std::string> cmnd) {
  if (!check_channel(main_fd, clients, cli_fd, channels, cmnd.at(1)))
    return ;
  if (cmnd.size() == 2) { 
    // WE ONLY RETURN THE TOPIC
    if (!channels[cmnd.at(1)]->get_topic_set())
      return send_error(main_fd, clients, cli_fd,":irc.ppeter 331 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + " :No topic is set");
    return send_error(main_fd, clients, cli_fd, ":irc.ppeter 332 " + clients[cli_fd]->get_nick() + space() + cmnd[1] + space() + channels[cmnd[1]]->get_topic());
  } else if (cmnd.size() >= 3) { // WE WANT TO SET THE TOPIC
    if (channels[cmnd[1]]->get_topic_set() && !check_op(main_fd, clients, cli_fd, channels, cmnd.at(1)))
      return ;
    channels[cmnd.at(1)]->set_topic(cmnd.at(2), clients[cli_fd]->get_nick());
    broadcast(main_fd, clients, cli_fd, channels, cmnd);
  }
}
