#include "../main.hpp"

std::string space( void ) {
  return " ";
}

void p_error( std::string err ) {
  std::cerr << "Error: " << err << " " << std::strerror(errno) << std::endl;
}

int ft_atoi( char *str ) {
  int port;

  char *end;
  errno = 0;
  long tmp = std::strtol(str, &end, 10);

  if (end == str)
    return p_error("Invalid port number."), 0;
  while (*end == ' ' || *end == '\t')
    ++end;
  if (*end != '\0')
    return p_error("Invalid port number."), 0;
  if ((errno == ERANGE && (tmp == LONG_MAX || tmp == LONG_MIN)) || tmp < INT_MIN || tmp > INT_MAX)
    return p_error("Invalid port number."), 0;
  port = static_cast<int>(tmp);
  if (port < 1024 || port > 65535)
        return p_error("Invalid port number."), 0;
  return port;
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
  printf("SEND: %s\n", error.c_str());
  clients[cli_fd]->set_out_buf(error);
  set_out(main_fd, cli_fd);
}

