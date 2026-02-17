#include <asm-generic/socket.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <vector>
#include <map>
#include <string>
#include "client.hpp"
#include "channel.hpp"
#include <algorithm>



#ifndef MAX_EPOLL_EVENTS
# define MAX_EPOLL_EVENTS 1024
#endif



char *str_join( char *str_1, char *str_2, int free_first ) {
  char *ret = NULL;
  int i = 0;
  int j = 0;

  if (!str_2)
    return NULL;
  while (str_1 && str_1[i])
    i++;
  while (str_2 && str_2[j])
    j++;
  ret = (char *)calloc(sizeof(char), i + j + 1);
  if (!ret) {
    perror("calloc() failed");
    return NULL;
  }
  i = 0;
  while (str_1 && str_1[i]) {
    ret[i] = str_1[i];
    i++;
  }
  j = 0;
  while (str_2 && str_2[j]) {
    ret[i + j] = str_2[j];
    j++;
  }
  if (free_first && str_1)
    free(str_1);
  return ret;
}

void send_error( int cli_fd, std::string error, int flags) {
  printf("\n\t\t\t\tSENDIN: %s$$$\n\n", error.c_str());
  send(cli_fd, (error + "\r\n").c_str(), error.size() + 2, flags); // MSG_DONTWAIT not needed here.
}

void disconnect_client( int main_fd, std::map<int, Client *> &clients, int cli_fd, std::string error) {
  send_error(cli_fd, error, 0);
  delete(clients[cli_fd]);
  clients.erase(cli_fd);
  epoll_ctl(main_fd, EPOLL_CTL_DEL, cli_fd, NULL);
  close(cli_fd);
  printf("PENIS0\n");
}

std::string space( void ) {
  return " ";
}

std::vector<std::string> buf_in( int main_fd, std::map<int, Client *> &clients, int cli_fd ) {
  std::vector<std::string> ret;

  std::string msg = clients[cli_fd]->get_buf();
  std::string another_buf;


  if (msg.empty())
    return ret;

  int i = msg.find("\r\n");
  if (i == std::string::npos) // not a valid line we 'wait' till next packet arrives
    return ret;

  if (i >= 511) { // a valid cmnd can't be longer then a message length without '\r\n'
    disconnect_client(main_fd, clients, cli_fd, "BRO THIS IS NOT CORRECT COMMAND.");
    return ret ;
  }

  std::string line(msg, 0, i);
  std::stringstream ss(line);
  std::string buf;

  while (getline(ss, buf, ' ')) {
    printf("PUSHED: %s$$$\n", buf.c_str());
    ret.push_back(buf);
  }

  int j = -1;
  int flag = 0;
  while (++j < ret.size() - 1) {
    if (ret.at(j)[0] == ':') {
      flag = 1;
      break ;
    }
  }

  std::string trailing;
  int k = j;

  while (flag && k < ret.size()) {
    trailing += space();
    trailing += ret.at(k++);
    printf("TRAILING: %s\n", trailing.c_str());
  }

  while (flag && k > j + 1) {
    printf("K: %d\n", k);
    ret.pop_back();
    k--;
  }

  if (!trailing.empty()) {
    ret.pop_back();
    ret.push_back(trailing);
  }

  for ( int i = 0; i < ret.size(); i++)
    printf("RET: %s\n", ret.at(i).c_str());

  // rest of msg in buf;
  i += 2;
  if (i < msg.size()) {
    std::string another_buf = msg.substr(i, msg.size() - i);
    clients[cli_fd]->set_buf(another_buf);
  } else
    clients[cli_fd]->clear_buf();
  return ret;
}

int nick_available(std::map<int, Client *> &clients, std::string nick) {
  for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); ++it) {
    if (it->second->get_nick() == nick)
      return 0;
  }
  return 1;
}


int registration(int main_fd, std::map<int, Client *> &clients, int cli_fd, std::vector<std::string> cmnd) {
  // REGISTRATION
  // TODO: what about TIMEOUT for pending registration ??
  if (cmnd.at(0) == "CAP") // for modern clients
    send_error(cli_fd, ":irc.ppeter.com CAP * LS", 0);
  else if (cmnd.at(0) == "PASS") {
    if (clients[cli_fd]->get_pass_set())
      send_error(cli_fd, ":irc.ppeter.com 462 " + clients[cli_fd]->get_nick() + " :Already registered.", 0);
    else if (cmnd.size() > 1 && printf("PASS: %s\n", clients[main_fd]->get_user().c_str()) && cmnd.at(1) == clients[main_fd]->get_user() && printf("SET PASS\n"))
      // PASS COMPLETE
      clients[cli_fd]->set_pass_set(true);
    else if (printf("SHOULD DISCONNECT\n"))
      return disconnect_client(main_fd, clients, cli_fd, ":irc.ppeter.com 464 " + clients[cli_fd]->get_nick() + " :Password mismatch."), 0;
    if (clients[cli_fd]->get_nick_set() && clients[cli_fd]->get_pass_set() && clients[cli_fd]->get_user_set() && !clients[cli_fd]->get_regi_set()) {
      clients[cli_fd]->set_regi_set(true);
      send_error(cli_fd, ":irc.ppeter.com 001 " + clients[cli_fd]->get_nick() + " :Welcome dickhead.", 0);
    }
  } else if (cmnd.at(0) == "USER") {
    printf("CMDN SIZE: %ld\n", cmnd.size());
    if (clients[cli_fd]->get_user_set())
      send_error(cli_fd, ":irc.ppeter.com 462 " + clients[cli_fd]->get_nick() + " :Already registered.", 0);
    if (cmnd.size() == 5 && printf("USER AT 4: %s\n", cmnd.at(4).c_str()) && cmnd.at(4)[0] == ':') {
      // USER COMPLETE
      clients[cli_fd]->set_user_set(true);
      clients[cli_fd]->set_user(cmnd.at(1));
      cmnd.at(4).erase(0, 1);
      clients[cli_fd]->set_real(cmnd.at(4));
      printf("SET USER: %s\n", cmnd.at(1).c_str());
    } else
      send_error(cli_fd, ":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + ":Not enough parameters.", 0);
    if (clients[cli_fd]->get_nick_set() && clients[cli_fd]->get_pass_set() && clients[cli_fd]->get_user_set() && !clients[cli_fd]->get_regi_set()) {
      clients[cli_fd]->set_regi_set(true);
      send_error(cli_fd, ":irc.ppeter.com 001 " + clients[cli_fd]->get_nick() + " :Welcome dickhead.", 0);
    }
  } else if (cmnd.at(0) == "NICK") {
    if (cmnd.size() > 1) { // TODO: CHECK IF CORRECT ORDER WHEN CHECKING CMND.SIZE()
      if (nick_available(clients, cmnd.at(1))) {
        // NICK COMPLETE
        clients[cli_fd]->set_nick(cmnd.at(1));
        clients[cli_fd]->set_nick_set(true);
      }
      else
        send_error(cli_fd, ":irc.ppeter.com 433 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + " :Nick already in use.", 0);
    }
    else
      send_error(cli_fd, ":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + ":Not enough parameters.", 0);
    if (clients[cli_fd]->get_nick_set() && clients[cli_fd]->get_pass_set() && clients[cli_fd]->get_user_set() && !clients[cli_fd]->get_regi_set()) {
      clients[cli_fd]->set_regi_set(true);
      send_error(cli_fd, ":irc.ppeter.com 001 " + clients[cli_fd]->get_nick() + " :Welcome dickhead.", 0);
    }
  } else if (!clients[cli_fd]->get_regi_set())
    return send_error(cli_fd, ":irc.ppeter.com 451 " + clients[cli_fd]->get_nick() + " :You have not registered.", 0), 1;
  else
    return -1;
  return 1;
}

// TODO: check for invalid access through wrong key in any map.
// TODO: set all const for classes

int valid_chars( std::string str ) {

  for (int i = 0; i < str.size(); i++) {
    if (str.at(i) <= 32 || str.at(i) >= 127 || str.at(i) == ',' || str.at(i) == ':')
      return 0;
  }
  return 1;
}

void broadcast( std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::vector<std::string> cmnd) {
  std::string channel = cmnd.at(1);
  std::map<int, std::string> channel_clients = channels[channel]->get_clients();
  // TODO: check for all instances like all commands + flags if this broadcast is working with cmnd.size() stuff
  for (std::map<int, std::string>::iterator it = channel_clients.begin(); it != channel_clients.end(); it++) {
    if (cmnd.size() == 4)
      send_error(it->first, clients[cli_fd]->get_prefix() + space() + cmnd.at(0) + space() + cmnd.at(1) + cmnd.at(2) + space() + cmnd.at(3), 0);
    else if (cmnd.size() == 3)
      send_error(it->first, clients[cli_fd]->get_prefix() + space() + cmnd.at(0) + space() + cmnd.at(1) + cmnd.at(2), 0);
    else
      send_error(it->first, clients[cli_fd]->get_prefix() + space() + cmnd.at(0) + space() + cmnd.at(1), 0);
  }
}

std::vector<std::string> names_list( Channel *channel ) {
  std::vector<std::string> list;
  std::map<int, std::string> clients = channel->get_clients();
  std::map<int, std::string> ops = channel->get_ops();

  for ( std::map<int, std::string>::iterator it = clients.begin(); it != clients.end(); it++ ) {
    std::map<int, std::string>::iterator sec = find(ops.begin(), ops.end(), *it);
    if (sec != ops.end())
      list.push_back("@" + it->second);
  }
  for ( std::map<int, std::string>::iterator it = clients.begin(); it != clients.end(); it++ ) {
    std::map<int, std::string>::iterator sec = find(ops.begin(), ops.end(), *it);
    if (sec == ops.end())
      list.push_back(it->second);
  }
  return list;
}

void send_from_cmnd( int cli_fd, std::string msg_part_1, std::vector<std::string> middle_msgs, std::string msg_part_2, int flags ) {
  std::vector<std::string>::iterator next_one;
  for (std::vector<std::string>::iterator it = middle_msgs.begin(); it != middle_msgs.end(); it++ ) {
    next_one = it + 1;
    if (it != middle_msgs.begin() && next_one != middle_msgs.end())
      msg_part_1.append(*it);
  }
  if (!msg_part_2.empty())
    msg_part_1.append(msg_part_2);
  send_error(cli_fd, msg_part_1, 0);
}

void send_annoying_error( int cli_fd, std::string msg_part_1, std::vector<std::string> middle_msgs, std::string msg_part_2, int flags ) {
  for (std::vector<std::string>::iterator it = middle_msgs.begin(); it != middle_msgs.end(); it++ ) {
    msg_part_1.append(*it);
  }
  if (!msg_part_2.empty())
    msg_part_1.append(msg_part_2);
  send_error(cli_fd, msg_part_1, 0);
}


void exec_JOIN(std::map<int, Client *> &clients, int cli_fd, std::vector<std::string> cmnd, std::map<std::string, Channel *> &channels) { 
  printf("EXECUTING JOIN CMND !!\n\n");
  if (clients[cli_fd]->get_channel_count() == 10)
    return send_error(cli_fd, ":irc.ppeter.com 405 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + ":You have joined too many channels.", 0);
  if (cmnd.at(1).empty() || cmnd.at(1)[0] != '#' || !valid_chars(cmnd.at(1)))
    return send_error(cli_fd,":irc.ppeter 476 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + " :Bad Channel Mask.", 0);
  try {
    if (channels.at(cmnd.at(1))->get_pass_set()) { // NOTE: this throws if channel does not exist yet. we leave it and do not use check_channel as JOIN is ONLY cmnd which allows creating one when channel does not exist yet.
      if (cmnd.size() < 2 || channels[cmnd.at(1)]->get_pass() != cmnd.at(2))
        return send_error(cli_fd,":irc.ppeter 475 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + " :Bad channel key.", 0);
    }
    // NOTE: we might as well put this in set_client of channels class
    if (channels[cmnd.at(1)]->get_user_count() >= channels[cmnd.at(1)]->get_limit() - 1)
      return send_error(cli_fd, ":irc:pperter.com 471 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + " :Channel is full.", 0);
    // JOINS THE CHANNEL
    // NOTE: WE REGISTER IN BOTH BUT IS THIS REALLY NECESSARY ??
    clients[cli_fd]->set_channel(cmnd.at(1));
    channels[cmnd.at(1)]->set_client(cli_fd, clients[cli_fd]->get_nick());
  } catch (const std::out_of_range &e) {
    // CREATES & JOINS THE CHANNEL AS OP
    Channel *new_one = new Channel();
    channels.emplace(cmnd.at(1), new_one);
    new_one->set_client(cli_fd, clients[cli_fd]->get_nick());
    clients[cli_fd]->set_channel(cmnd.at(1));
    new_one->set_name(cmnd.at(1));
    printf("NEWLY ADDED CHANNEL: %s$\n", channels[cmnd.at(1)]->get_name().c_str());
    new_one->set_op(cli_fd, clients[cli_fd]->get_nick());
  }
  broadcast(clients, cli_fd, channels, cmnd);
  if (channels[cmnd.at(1)]->get_topic_set()) {
    send_error(cli_fd,":irc.ppeter 332 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + space() + ":" + channels[cmnd.at(1)]->get_topic(), 0), 0;
    // send_error(cli_fd,":irc.ppeter 332 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + space() + channels[cmnd.at(1)]->get_setter_nick() + space() + channels.at(cmnd.at(1))->get_topic_timestamp(), 0), 0;
  } else
    send_error(cli_fd,":irc.ppeter 331 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + " :No topic is set", 0), 0;
  send_annoying_error(cli_fd, ":irc.ppeter 353 " + clients[cli_fd]->get_nick() + space() + "=" + space() + cmnd.at(1) + space() + ":", names_list(channels[cmnd.at(1)]), "", 0);
  send_error(cli_fd,":irc.ppeter 366 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + " :End of /NAMES list", 0);
}

int check_channel(std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::vector<std::string> cmnd) {
  std::string channel = cmnd.at(1);
  try {
    if (channels.at(channel)) {
      if (!channels.at(channel)->check_client(cli_fd))
        return send_error(cli_fd, ":irc.ppeter.com 442 " + clients[cli_fd]->get_nick() + space() + cmnd.at(2) + ":You are not on this channel.", 0), 0;
      return 1;
    }
  } catch (const std::out_of_range &e) {
    return send_error(cli_fd,":irc.ppeter 403 " + clients[cli_fd]->get_nick() + space() + cmnd.at(2) + " :No such channel.", 0), 0;
  }
  return 0;
}

int check_op(std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::vector<std::string> cmnd) {
  if (!check_channel(clients, cli_fd, channels, cmnd))
      return 0;
  if (!channels.at(cmnd.at(1))->check_op(cli_fd))
    return send_error(cli_fd,":irc.ppeter 482 " + clients[cli_fd]->get_nick() + space() + cmnd.at(2) + " :Channel operator privileges needed.", 0), 0;
  return 1;
}


void exec_TOPIC( std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::vector<std::string> cmnd) {
  if (!check_channel(clients, cli_fd, channels, cmnd))
    return ;
  if (cmnd.size() == 2) { 
    // WE ONLY RETURN THE TOPIC
    if (!channels[cmnd.at(1)]->get_topic_set())
      return send_error(cli_fd,":irc.ppeter 331 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + " :No topic is set", 0);
    return send_error(cli_fd, ":irc::ppeter.com 332 " + clients[cli_fd]->get_nick() + space() + cmnd[1] + channels[cmnd[1]]->get_topic(), 0);
  } else if (cmnd.size() >= 3) { // WE WANT TO SET THE TOPIC
    if (channels[cmnd[1]]->get_topic_set() && !check_op(clients, cli_fd, channels, cmnd))
      return ;
    if (cmnd.at(2)[0] == ':')
      cmnd.at(2).erase(0, 1);
    // WE SET THE TOPIC
    channels[cmnd.at(1)]->set_topic(cmnd.at(2), clients[cli_fd]->get_nick());
    broadcast(clients, cli_fd, channels, cmnd);
  }
}

int check_client( std::map<int, Client *> &clients, std::string nick ) {
  for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); it++) {
    if (it->second->get_nick() == nick) {
      return it->second->get_fd();
    }
  }
  return 0;
}


void exec_INVITE( std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::vector<std::string> cmnd) {
// INVITE: <nickname> <channel>
  if (cmnd.size() <= 2)
    return send_error(cli_fd, ":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + ":Not enough parameters.", 0);
  if (!nick_available(clients, cmnd.at(1)))
    return send_error(cli_fd, ":irc.ppeter.com 401 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + ":No such nick.", 0);
  if (!check_channel(clients, cli_fd, channels, cmnd))
    return ;
  if (!channels[cmnd.at(2)]->check_client(cli_fd))
    return send_error(cli_fd, ":irc.ppeter.com 442 " + clients[cli_fd]->get_nick() + space() + cmnd.at(2) + ":You are not on this channel.", 0);
  if (channels[cmnd.at(2)]->check_client(check_client(clients, cmnd.at(1))))
    return send_error(cli_fd, (":irc.ppeter.com 443 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + space() + cmnd.at(2) + " :User is already on this channel.").c_str(), 0);
  if (channels[cmnd.at(2)]->get_invite_set() && !check_op(clients, cli_fd, channels, cmnd))
    return ;
  send_error(check_client(clients, cmnd.at(1)), (clients[cli_fd]->get_prefix() + space() + cmnd.at(0) + space() + cmnd.at(1) + " :" + cmnd.at(2)).c_str(), 0);
  send_error(cli_fd, (":irc.ppeter.com 341 " + clients[cli_fd]->get_nick() + cmnd.at(1) + cmnd.at(2).c_str()), 0);
}


void exec_MSG( std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::vector<std::string> cmnd) {
  if (cmnd.size() == 2 && cmnd.at(1)[0] == ':')
    return send_error(cli_fd, (":irc.ppeter.com 411 " + clients[cli_fd]->get_nick() + " :No recipient given (PRIVMSG)"), 0);
  if (cmnd.size() == 2)
    return send_error(cli_fd, (":irc.ppeter.com 412 " + clients[cli_fd]->get_nick() + " :No text to send."), 0);
  if (cmnd.size() > 3)
    return send_from_cmnd(cli_fd, ":irc.ppeter.com 407 " + clients[cli_fd]->get_nick(), cmnd, " :Too many recipients.", 0);
  if (cmnd.at(1)[0] == '#') {
    if (!check_channel(clients, cli_fd, channels, cmnd))
      return send_error(cli_fd, (":irc.ppeter.com 401 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + " :No such nick."), 0);
    // SENDING TO CHANNEL..
    broadcast(clients, cli_fd, channels, cmnd);
  } else {
    if (!check_client(clients, cmnd.at(1)))
      return send_error(cli_fd,":irc.ppeter 403 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + " :No such channel.", 0);
    // SENDING TO CLIENT..
    send_error(check_client(clients, cmnd.at(1)), ":" + clients[cli_fd]->get_nick() + "!" + clients[cli_fd]->get_user() + "@" + clients[cli_fd]->get_host() + space() + cmnd.at(0) + space() + cmnd.at(2), 0);
  }
}

void exec_KICK( std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::vector<std::string> cmnd) {
  if ((cmnd.size() < 3) || (cmnd.size() == 3 && cmnd.at(2)[0] == ':') || (cmnd.at(cmnd.size() - 1)[0] != ':'))
    return send_error(cli_fd, (":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters.").c_str(), 0);
  if (!check_channel(clients, cli_fd, channels, cmnd))
    return ;
  if (!check_op(clients, cli_fd, channels, cmnd))
    return ;
  // KICK
  channels[cmnd.at(1)]->drop_client(check_client(clients, cmnd.at(2)));
  broadcast(clients, cli_fd, channels, cmnd);
}

std::string current_modes(Channel *channel)
{
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


void exec_MODE( std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::vector<std::string> cmnd) {
  if (!check_channel(clients, cli_fd, channels, cmnd))
    return ;
  if (!check_op(clients, cli_fd, channels, cmnd))
    return ;
  // TODO: CHECK ORDER WITH EACH INDIVIDUAL 'Not enought parameters.'
  if (cmnd.size() == 2) {
      return send_error(cli_fd, ":irc.ppeter.com 324 " + clients[cli_fd]->get_nick() + space() + cmnd.at(1) + space() + current_modes(channels.at(cmnd.at(1))), 0);
  } else if (cmnd.at(2) == "+i" || cmnd.at(2) == "-i") {
    if (cmnd.size() != 3)
      return send_error(cli_fd, (":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters."), 0);
    if (cmnd.at(2) == "+i")
      channels[cmnd.at(1)]->set_invite_set(true);
    else
      channels[cmnd.at(1)]->set_invite_set(false);
  } else if ( cmnd.at(2) == "+t" || cmnd.at(2) == "-t") {
    if (cmnd.size() != 3)
      return send_error(cli_fd, (":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters.").c_str(), 0);
    if (cmnd.at(2) == "+i")
      channels[cmnd.at(1)]->set_topic_set(true);
    else
      channels[cmnd.at(1)]->set_topic_set(false);
  } else if ( cmnd.at(2) == "+k" || cmnd.at(2) == "-k") {
    if (cmnd.at(2) == "+k" && cmnd.size() != 4)
      return send_error(cli_fd, (":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters.").c_str(), 0);
    else if (cmnd.at(2) == "-k" && cmnd.size() != 3)
      return send_error(cli_fd, (":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters.").c_str(), 0);
    if (cmnd.at(2) == "+k")
      channels[cmnd.at(1)]->set_pass(cmnd.at(3));
    else
      channels[cmnd.at(1)]->unset_pass();
  } else if ( cmnd.at(2) == "+o" || cmnd.at(2) == "-o") {
    if (cmnd.size() != 4)
      return send_error(cli_fd, (":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters.").c_str(), 0);
    if (!check_client(clients, cmnd.at(3)))
      return send_error(cli_fd, ":irc.ppeter.com 441 " + clients[cli_fd]->get_nick() + space() + cmnd.at(2) + ":User is not on this channel.", 0);
    if (!channels[cmnd.at(1)]->check_client(check_client(clients, cmnd.at(3))))
      return ;
    // SET OP
    channels[cmnd.at(1)]->set_op(check_client(clients, cmnd.at(3)), cmnd.at(3));
  } else if ( cmnd.at(2) == "+l" || cmnd.at(2) == "-l") {
    if (cmnd.at(2) == "+l" && cmnd.size() != 4)
      return send_error(cli_fd, (":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters.").c_str(), 0);
    else if (cmnd.at(2) == "-l" && cmnd.size() != 3)
      return send_error(cli_fd, (":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters.").c_str(), 0);
    if (cmnd.at(2) == "+l" && !channels[cmnd.at(1)]->set_limit(cmnd.at(3))) // SET LIMIT
      return send_error(cli_fd, (":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters.").c_str(), 0);
    else if (cmnd.at(2) == "-l")
      channels[cmnd.at(1)]->unset_limit();
  }
  broadcast(clients, cli_fd, channels, cmnd);
}

void exec_other(int main_fd, std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::vector<std::string> cmnd) {

  // NOTE: ORIGINALLY THIS SHOULD BE INSIDE EVERY COMMAND AFTER BASIC COMMAND COMPATIBILITY IS CHECKED, BUT.. WHO CARES.
  if (cmnd.size() <= 1)
    return send_error(cli_fd, (":irc.ppeter.com 461 " + clients[cli_fd]->get_nick() + space() + cmnd.at(0) + " :Not enough parameters.").c_str(), 0);
  if (cmnd.at(0) == "JOIN") {
    exec_JOIN(clients, cli_fd, cmnd, channels);
  } else if (cmnd.at(0) == "PRIVMSG") {
    exec_MSG(clients, cli_fd, channels, cmnd);
  } else if (cmnd.at(0) == "KICK") {
    exec_KICK(clients, cli_fd, channels, cmnd);
  } else if (cmnd.at(0) == "INVITE") {
    exec_INVITE(clients, cli_fd, channels, cmnd);
  } else if (cmnd.at(0) == "TOPIC") {
    exec_TOPIC(clients, cli_fd, channels, cmnd);
  } else if (cmnd.at(0) == "MODE") {
    exec_MODE(clients, cli_fd, channels, cmnd);
  }
}

void exec_cmnd(int main_fd, std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels) {

  std::vector<std::string> cmnd;
  cmnd = buf_in(main_fd, clients, cli_fd);

  if (cmnd.empty())
    return ;

  printf("CMND: %s\n", cmnd.at(0).c_str());
  int regi = registration(main_fd, clients, cli_fd, cmnd);
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


  struct sockaddr_in serv_addr, client_addr;

  std::map<int, Client *> clients;

  socklen_t cli_len;
  cli_len = sizeof(client_addr);

  int port;
  port = htons(atoi(argv[1]));

  int main_fd, sockfd;
  std::string pass = argv[2];

  // fuck we also need all the channels available for everyone;
  std::map<std::string, Channel *>channels;


  main_fd = epoll_create1(0);
  if (main_fd == -1)
    perror("epoll_create1() failed");
  Client *non_client = new Client();
  clients.emplace(main_fd, non_client);
  clients[main_fd]->set_user(pass);

  // connection-less socket means [ like UDP ]:
  // - no connect() or similar; just send data
  // - each message is sent independent
  //    - every msg includes dest. address
  // - no deliver guarantee
  // - no built in reliability or flow control
  //    - no ack
  //    - no retry
  //    - no ordering
  //    - no congestion management
  //    - all you must do yourself
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1)
    perror("socket() failed");

  // operates on the file descriptor
  // affects I/O semantics
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
  int opt = 1;

  // operates on the socket option level SOL
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // so we can restart server immediately [ Allows rebinding to a port that is in TIME_WAIT ( port gets added to TIME_WAIT and is thereby blocked ) ].

  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = port;
  serv_addr.sin_addr.s_addr = INADDR_ANY; // ACCEPT ALL CONNECTION ORIGINS

  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    perror("bind() failed");

  listen(sockfd, SOMAXCONN);

  struct epoll_event event;
  struct epoll_event events[MAX_EPOLL_EVENTS];

  event.events = EPOLLIN;
  event.data.fd = sockfd;

  if (epoll_ctl(main_fd, EPOLL_CTL_ADD, sockfd, &event) == -1)
    perror("epoll_ctl() failed");

  char buf[513];

  while(true) {
    int n_ev = epoll_wait(main_fd, events, MAX_EPOLL_EVENTS, -1);

    if (n_ev == -1) {
      perror("epoll_wait() failed");
      return 1 ;
    }
    for (int i = 0; i < n_ev; i++) {
      if (events[i].data.fd == sockfd) {
        int cli_sock = -1;
        while (true) {
          cli_sock = accept(sockfd, (struct sockaddr *) &client_addr, &cli_len);
          if (cli_sock == -1 && errno != EAGAIN) {
             perror("accept() failed");
             return 1 ;
          } else if (cli_sock == -1)
            break ;
          int flags = fcntl(cli_sock, F_GETFL, 0);
          fcntl(cli_sock, F_SETFL, flags | O_NONBLOCK);
          event.events = EPOLLIN | EPOLLRDHUP;
          event.data.fd = cli_sock;
          printf("NEW CLIENT \n");
          if (epoll_ctl(main_fd, EPOLL_CTL_ADD, cli_sock, &event) == -1) {
            perror("epoll_ctl() failed.");
            return 1 ;
          }
          Client *new_one = new Client();
          clients.insert({cli_sock, new_one});
          new_one->set_host(client_addr.sin_addr.s_addr);
        }
      } else {
        int cli_fd = events[i].data.fd;

        printf("CLIENT fd=%d ev=%x\n", cli_fd, events[i].events);

        uint32_t ev = events[i].events;
        // EPOLLHUP
        // HUP --> HANGUP
        // - peer closed
        // - local shutdown
        // - connection teardown
        // Note: connection is FULLY GONE
        // EPOLLRDHUP ( only write side of client is closed )
        // - client closed write side
        // - TCP FIN received
        // - half closed
        // Note: still can be unread data, and still one way direction is open
        if (ev & (EPOLLERR | EPOLLHUP )) {
          epoll_ctl(main_fd, EPOLL_CTL_DEL, cli_fd, NULL); // stop monitoring the client fd.
          close(cli_fd); // NOTE: FIRST DEL THEN CLOSE.
          continue ;
        }


        std::string msg;
        int n = 0;
        while (true) {
          // NOTE: DONT USE READ HERE
          //        --> recv() allows socket specific control via flags --> NON_BLOCKING: MSG_DONTWAIT BUT FD IS ALREADY SET TO O_NON_BLOCKING
          n = recv(cli_fd, buf, sizeof(buf) - 1, 0);
          if (n == -1 && errno == EAGAIN && printf("EAGAIN\n"))
            break ;
          if (n <= 0) {
            printf("NULL BYTES READ\n");
            disconnect_client(main_fd, clients, cli_fd, "WE NEED AN ERROR HERE");
            break ;
          } else if (n <= 0 && printf("WE BREAK: NOTHING TO READ ANYMORE\n"))
            break ;
          buf[n] = 0;
          msg.append(buf);
          printf("MSG: %s\n", msg.c_str());
        }
        printf("MSG: %s || %d \n", msg.c_str(), n);
        if (n == 0) {
          delete (clients[cli_fd]);
          delete (clients[main_fd]);
          return 0 ;
        }
        printf("PENIS1\n");
        clients[cli_fd]->set_buf(msg);
        printf("exec_cmnd: %s\n", clients[cli_fd]->get_buf().c_str());
        exec_cmnd(main_fd, clients, cli_fd, channels);
      }
    }
  }


  return 0;
}


