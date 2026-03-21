#include "../main.hpp"

std::string space( void ) {
  return " ";
}

void p_error( std::string err ) {
  std::cerr << "Error: " << err << " " << std::strerror(errno) << std::endl;
}

void set_out( int main_fd, int cli_fd ) {
  struct epoll_event ev;
  ev.data.fd = cli_fd;
  ev.events = EPOLLIN | EPOLLRDHUP | EPOLLOUT;
  if (epoll_ctl(main_fd, EPOLL_CTL_MOD, cli_fd, &ev) == -1)
    p_error("epoll_ctl() failed ");
}

int unset_out( int main_fd, int cli_fd ) {
  struct epoll_event ev;
  ev.data.fd = cli_fd;
  ev.events = EPOLLIN | EPOLLRDHUP;
  if (epoll_ctl(main_fd, EPOLL_CTL_MOD, cli_fd, &ev) == -1)
    return p_error("epoll_ctl() failed "), 0;
  return 1;
}

void send_error( int main_fd, std::map<int, Client *> &clients, int cli_fd, std::string error) {
  clients[cli_fd]->set_out_buf(error);
  set_out(main_fd, cli_fd);
}

