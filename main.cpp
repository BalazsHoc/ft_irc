#include "main.hpp"

volatile sig_atomic_t g_signal = 0;


void  signalHandler(int signum) {
  if (signum == SIGINT) {
    g_signal = 1;
    std::cout << "\nCaught Ctrl+C (SIGINT). Shutting down...\n";
  }
}


std::vector<std::string> buf_in(int main_fd, std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels) {
  std::vector<std::string> ret;

  std::string msg = clients[cli_fd]->get_in_buf();

  if (msg.empty())
    return ret;

  int i = msg.find("\r\n");

  if (i >= 510) { // a valid cmnd can't be longer then a message length without '\r\n'
    send_error(main_fd, clients, cli_fd, "ERROR :Closing Link: " + clients[cli_fd]->get_host() + " (Line too long)");
    disconnect_client(channels, main_fd, clients, cli_fd);
    return ret ;
  }

  if (i < 0 || i == (int)std::string::npos) // not a valid line we 'wait' till next packet arrives
    return ret;

  std::string line(msg, 0, i);
  std::stringstream ss(line);
  std::string buf;

  while (std::getline(ss, buf, ' ')) {
    ret.push_back(buf);
  }

  int j = -1;
  int flag = 0;
  while (!ret.empty() && ++j < (int)ret.size() - 1) {
    if (ret.at(j)[0] == ':') {
      flag = 1;
      break ;
    }
  }

  std::string trailing;
  int k = j;

  while (flag && k < (int)ret.size()) {
    if (k != j)
      trailing += space();
    trailing += ret.at(k++);
  }

  while (flag && k > j + 1) {
    ret.pop_back();
    k--;
  }

  if (!trailing.empty()) {
    ret.pop_back();
    ret.push_back(trailing);
  }

  // rest of msg in buf;
  i += 2;
  if (i < (int)msg.size()) {
    std::string another_buf = msg.substr(i, msg.size() - (i - 2));
    clients[cli_fd]->set_in_buf(another_buf);
  } else
    clients[cli_fd]->clear_in_buf();
  return ret;
}


int registration(int main_fd, std::map<int, Client *> &clients, int cli_fd, std::vector<std::string> cmnd, std::map<std::string, Channel *> &channels) {
  // REGISTRATION
  if (cmnd.at(0) == "CAP") // for modern clients
    send_error(main_fd, clients, cli_fd, ":irc.ppeter.com CAP * LS :");
  if (cmnd.at(0) == "PING" && cmnd.size() > 1) // for modern clients
    send_error(main_fd, clients, cli_fd, ":irc.ppeter.com PONG :" + cmnd.at(1));
  else if (cmnd.at(0) == "PASS") {
    if (clients[cli_fd]->get_pass_set())
      send_error(main_fd, clients, cli_fd, ":irc.ppeter.com 462 " + clients[cli_fd]->get_nick() + " :Already registered.");
    else if (!is_valid_char(cmnd.at(1)))
      send_error(main_fd, clients, cli_fd, ":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters.");
    else if ((int)cmnd.size() > 1 && cmnd.at(1) == clients[main_fd]->get_user())
      // PASS COMPLETE
      clients[cli_fd]->set_pass_set(true);
    else
      return send_error(main_fd, clients, cli_fd, ":irc.ppeter.com 464 " + clients[cli_fd]->get_nick() + " :Password mismatch."),
             disconnect_client(channels, main_fd, clients, cli_fd), 0;
    if (clients[cli_fd]->get_nick_set() && clients[cli_fd]->get_pass_set() && clients[cli_fd]->get_user_set() && !clients[cli_fd]->get_regi_set()) {
      clients[cli_fd]->set_regi_set(true);
      send_error(main_fd, clients, cli_fd, ":irc.ppeter.com 001 " + clients[cli_fd]->get_nick() + " :Welcome dickhead.");
    }
  } else if (cmnd.at(0) == "USER") {
    // USER <username> <hostname> <servername> :<realname>
    if (clients[cli_fd]->get_user_set())
      send_error(main_fd, clients, cli_fd, ":irc.ppeter.com 462 " + clients[cli_fd]->get_nick() + " :Already registered.");
    else if (cmnd.size() == 5 && cmnd.at(4)[0] == ':' && is_valid_char(cmnd.at(1))) {
      // USER COMPLETE
      clients[cli_fd]->set_user_set(true);
      clients[cli_fd]->set_user(cmnd.at(1));
      cmnd.at(4).erase(0, 1);
      clients[cli_fd]->set_real(cmnd.at(4));
    } else
      send_error(main_fd, clients, cli_fd, ":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters.");
    if (clients[cli_fd]->get_nick_set() && clients[cli_fd]->get_pass_set() && clients[cli_fd]->get_user_set() && !clients[cli_fd]->get_regi_set()) {
      clients[cli_fd]->set_regi_set(true);
      send_error(main_fd, clients, cli_fd, ":irc.ppeter.com 001 " + clients[cli_fd]->get_nick() + " :Welcome dickhead.");
    }
  } else if (cmnd.at(0) == "NICK") {
    if (cmnd.size() > 1) {
      if (!is_valid_nick(cmnd.at(1)))
        send_error(main_fd, clients, cli_fd, ":irc.ppeter.com 432 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + " :Erroneous nickname.");
      else if (nick_available(clients, cmnd.at(1))) {
        // NICK COMPLETE
        clients[cli_fd]->set_nick(cmnd.at(1));
        clients[cli_fd]->set_nick_set(true);
      }
      else
        send_error(main_fd, clients, cli_fd, ":irc.ppeter.com 433 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + " :Nick already in use.");
    }
    else
      send_error(main_fd, clients, cli_fd, ":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters.");
    if (clients[cli_fd]->get_nick_set() && clients[cli_fd]->get_pass_set() && clients[cli_fd]->get_user_set() && !clients[cli_fd]->get_regi_set()) {
      clients[cli_fd]->set_regi_set(true);
      send_error(main_fd, clients, cli_fd, ":irc.ppeter.com 001 " + clients[cli_fd]->get_nick() + " :Welcome dickhead.");
    }
  } else if (!clients[cli_fd]->get_regi_set())
    return send_error(main_fd, clients, cli_fd, ":irc.ppeter.com 451 " + clients[cli_fd]->get_nick() + " :You have not registered."), 1;
  else
    return -1;
  return 1;
}

void exec_other(int main_fd, std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::vector<std::string> cmnd) {

  if (cmnd.size() <= 1)
    return send_error(main_fd, clients, cli_fd, (":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters.").c_str());
  if (cmnd.at(0) == "JOIN") {
    exec_JOIN(main_fd, clients, cli_fd, cmnd, channels);
  } else if (cmnd.at(0) == "PRIVMSG") {
    exec_MSG(main_fd, clients, cli_fd, channels, cmnd);
  } else if (cmnd.at(0) == "KICK") {
    exec_KICK(main_fd, clients, cli_fd, channels, cmnd);
  } else if (cmnd.at(0) == "INVITE") {
    exec_INVITE(main_fd, clients, cli_fd, channels, cmnd);
  } else if (cmnd.at(0) == "TOPIC") {
    exec_TOPIC(main_fd, clients, cli_fd, channels, cmnd);
  } else if (cmnd.at(0) == "MODE") {
    exec_MODE(main_fd, clients, cli_fd, channels, cmnd);
  }
}

void exec_cmnd(int main_fd, std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels) {

  std::vector<std::string> cmnd;
  cmnd = buf_in(main_fd, clients, cli_fd, channels);

  if (cmnd.empty())
    return ;

  if (!is_valid_cmnd(cmnd.at(0)))
    return send_error(main_fd, clients, cli_fd, (":irc.ppeter.com 421 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Unknown command.").c_str());
  int regi = registration(main_fd, clients, cli_fd, cmnd, channels);
  if (!regi)
    return ;
  if (regi == -1)
    exec_other(main_fd, clients, cli_fd, channels, cmnd);
  exec_cmnd(main_fd, clients, cli_fd, channels);
}


int main( int argc, char **argv ) {

  if (argc != 3) {
    std::cout << "usage " << argv[0] << " <port> <password>" << std::endl;
    return 0;
  }

  std::signal(SIGINT, signalHandler);

  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGPIPE, &sa, NULL);

  struct sockaddr_in serv_addr = {};
  struct sockaddr_in client_addr = {};

  std::map<int, Client *> clients;

  socklen_t cli_len;
  cli_len = sizeof(client_addr);

  int port;
  port = valid_port(argv[1]);
  if (port == 0)
    return p_error("No valid port."), 0;
  port = htons(port);

  int main_fd, sockfd;
  std::string pass = argv[2];
  if (!is_valid_char(pass))
    return perror("No valid pass."), 0;

  std::map<std::string, Channel *>channels;

  main_fd = epoll_create1(0);
  if (main_fd == -1) {
    p_error("epoll_create1() failed ");
    return 1;
  }
  Client *non_client = new Client(main_fd);
  clients[main_fd] = non_client;
  clients[main_fd]->set_user(pass);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1)
    return p_error("socket() failed "), disconnect_main(main_fd, clients, channels, main_fd), 1;

  fcntl(sockfd, F_SETFL, O_NONBLOCK);
  int opt = 1;

  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = port;
  serv_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    return p_error("bind() failed "), disconnect_main(main_fd, clients, channels, main_fd), 1;

  if (listen(sockfd, SOMAXCONN) == -1)
    return p_error("listen() failed "), disconnect_main(main_fd, clients, channels, main_fd), 1;

  struct epoll_event event;
  struct epoll_event events[MAX_EPOLL_EVENTS];

  event.events = EPOLLIN;
  event.data.fd = sockfd;

  if (epoll_ctl(main_fd, EPOLL_CTL_ADD, sockfd, &event) == -1) {
    p_error("epoll_ctl() failed ");
    return 1;
  }

  char buf[513];

  while(!g_signal) {
    int n_ev = epoll_wait(main_fd, events, MAX_EPOLL_EVENTS, -1);
    if (n_ev == -1 && !g_signal) {
      p_error("epoll_wait() failed. ");
      disconnect_client(channels, main_fd, clients, main_fd);
      return 1 ;
    }
    for (int i = 0; i < n_ev; i++) {
      if (events[i].data.fd == sockfd) {
        int cli_sock = -1;
        if (events[i].events & EPOLLIN) {
          cli_sock = accept(sockfd, (struct sockaddr *) &client_addr, &cli_len);
          if (cli_sock == -1 && !g_signal) {
            p_error("accept() failed. ");
            break ;
          }
          fcntl(cli_sock, F_SETFL, O_NONBLOCK);
          event.events = EPOLLIN | EPOLLRDHUP;
          event.data.fd = cli_sock;
          if (epoll_ctl(main_fd, EPOLL_CTL_ADD, cli_sock, &event) == -1 && !g_signal) {
            p_error("epoll_ctl() failed. ");
            disconnect_main(main_fd, clients, channels, cli_sock);
            return 1 ;
          }
          Client *new_one = new Client(cli_sock);
          new_one->set_host(client_addr.sin_addr.s_addr);
          clients[cli_sock] = new_one;
        }
      } else {
        int cli_fd = events[i].data.fd;


        uint32_t ev = events[i].events;
        if (ev & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
          disconnect_client(channels, main_fd, clients, cli_fd);
          break ;
        }


        int n = -1;
        if (g_signal) {
          disconnect_main(main_fd, clients, channels, sockfd);
          return 1;
        }
        if (events[i].events & EPOLLIN) {
          n = recv(cli_fd, buf, sizeof(buf) - 1, 0);
          if (n <= 0) {
            disconnect_client(channels, main_fd, clients, cli_fd);
            continue ;
          } else {
            clients[cli_fd]->append_to_buf(buf);
            exec_cmnd(main_fd, clients, cli_fd, channels);
            std::fill(buf, buf + sizeof(buf), 0);
          }
        } else if (events[i].events & EPOLLOUT) {
          clients[cli_fd]->send_out();
          if (clients[cli_fd]->get_out_buf().empty() && !unset_out(main_fd, cli_fd))
            disconnect_client(channels, main_fd, clients, cli_fd);
        }
      }
    }
  }

  if (g_signal) {
    disconnect_main(main_fd, clients, channels, sockfd);
    return 1;
  }

  return 0;
}


