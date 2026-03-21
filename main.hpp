#ifndef MAIN_HPP

#define MAIN_HPP

#include <asm-generic/socket.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
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
#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>

#include <csignal>


#include "client.hpp"
#include "channel.hpp"


#ifndef MAX_EPOLL_EVENTS
# define MAX_EPOLL_EVENTS 1024
#endif


// utils.cpp


std::string     space( void );
void            p_error( std::string err );
void            set_out( int main_fd, int cli_fd );
int             unset_out( int main_fd, int cli_fd );
void            send_error( int main_fd, std::map<int, Client *> &clients, int cli_fd, std::string error);

// disconnecting.cpp
void            disconnect_client( std::map<std::string, Channel *> &channels, int main_fd, std::map<int, Client *> &clients, int cli_fd );
void            disconnect_main( int main_fd, std::map<int, Client *> &clients, std::map<std::string, Channel *> &channels, int sock_fd );



//  validation.cpp

int             is_alpha( char c );
int             is_num( char c );
int             is_special ( char c );
int             is_valid_nick( std::string nick );
int             is_valid_char( std::string user );
int             valid_chars( std::string str );

//  check_class.cpp

int             check_channel(int main_fd, std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::string channel);
int             check_op(int main_fd, std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::string channel);
int             check_client( std::map<int, Client *> &clients, std::string nick );
int             nick_available(std::map<int, Client *> &clients, std::string nick);

//  msg.cpp

void                        broadcast( int main_fd, std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::vector<std::string> cmnd);
std::vector<std::string>    names_list( Channel *channel );
std::string                 send_from_cmnd( std::string msg_part_1, std::vector<std::string> middle_msgs, std::string msg_part_2 );
std::string                 send_annoying_error( std::string msg_part_1, std::vector<std::string> middle_msgs, std::string msg_part_2 );


//  COMMANDS

void                exec_INVITE( int main_fd, std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::vector<std::string> cmnd);
void                exec_JOIN(int main_fd, std::map<int, Client *> &clients, int cli_fd, std::vector<std::string> cmnd, std::map<std::string, Channel *> &channels);
void                exec_KICK( int main_fd, std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::vector<std::string> cmnd);
void                exec_MODE( int main_fd, std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::vector<std::string> cmnd);
void                exec_MSG( int main_fd, std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::vector<std::string> cmnd);
void                exec_TOPIC( int main_fd, std::map<int, Client *> &clients, int cli_fd, std::map<std::string, Channel *> &channels, std::vector<std::string> cmnd);


#endif
