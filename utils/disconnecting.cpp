#include "../main.hpp"

void disconnect_client( std::map<std::string, Channel *> &channels, int main_fd, std::map<int, Client *> &clients, int cli_fd ) {

  std::string *chan_arr = clients[cli_fd]->get_channels();
  int chan_count = clients[cli_fd]->get_channel_count();

  for (int i = 0; i < chan_count; i++) {
    channels.at(chan_arr[i])->unset_cli(cli_fd);
    if (channels.at(chan_arr[i])->get_user_count() <= 0) {
      delete channels.at(chan_arr[i]);
      channels.erase(chan_arr[i]);
    }
  }
  if (epoll_ctl(main_fd, EPOLL_CTL_DEL, cli_fd, NULL) == -1) {
    p_error("epoll_ctl() failed ");
  }
  close(cli_fd);
  std::map<int, Client *>::iterator it = clients.find(cli_fd);
  if (it != clients.end()) {
    delete it->second;
    clients.erase(it);
  }
}

void disconnect_main( int main_fd, std::map<int, Client *> &clients, std::map<std::string, Channel *> &channels, int sock_fd ) {
  // FIRST DISONNECT ALL CLIENTS
  for ( std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ) {
    std::map<int, Client *>::iterator prev = it++;
    if (prev->first != main_fd)
      disconnect_client(channels, main_fd, clients, prev->first);
  }
  for ( std::map<std::string, Channel *>::iterator it = channels.begin(); it != channels.end(); ) {
    std::map<std::string, Channel *>::iterator prev = it++;
    delete prev->second;
    channels.erase(prev->first);
  }
  close(main_fd);
  delete(clients[main_fd]);
  clients.erase(main_fd);
  if (main_fd != sock_fd)
    close(sock_fd);
}
