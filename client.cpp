#include "client.hpp"
#include <string.h>


Client::Client ( void ) {
  _nick = "*";
  _channel_count = 0;
}


// Client::Client ( Client & orig ) : _user(orig._user), _channel(orig._channel), _buf(orig._buf) {}

// Client::Client & operator = ( Client & orig );

Client::~Client ( ) {
}

void Client::set_pass_set( bool value ) {
  _pass_set = value;
}

void Client::set_regi_set( bool value ) {
  _regi_set = value;
}

void Client::set_user_set( bool value ) {
  _user_set = value;
}

void Client::set_nick_set( bool value ) {
  _nick_set = value;
}

void Client::set_user( std::string name ) {
  _user = name;
}

void Client::set_real( std::string realname ) {
  _realname = realname;
}

void Client::set_nick( std::string nick ) {
  _nick = nick;
}

void Client::set_host( unsigned long ip_addr ) { // network-bite order
  char buf[INET_ADDRSTRLEN]; // ipv4
  inet_ntop(AF_INET, &ip_addr, buf, sizeof(buf));
  _host = std::string(buf);
}

void Client::set_buf( std::string buf ) {
  _buf = buf;
}

void Client::clear_buf( void ) {
  _buf = "";
}

// void Client::set_channel( Channel *channel ) {
//   _channel[_channel_count] = channel;
//   _channel_count++;
// }

void Client::set_channel( std::string channel ) {
  if (_channel_count >= _channel_limit) // not really needed but extra safe
    return ;
  for (int i = 0; i < _channel_count; i++) {
    if (_channel[i] == channel)
      return ;
  }
  _channel[_channel_count] = channel;
  _channel_count++;
}


void Client::unset_invite( std::string channel ) {
  std::vector<std::string>::iterator it = std::find(_invited.begin(), _invited.end(), channel);
  if (it != _invited.end())
    _invited.erase(it);
}

bool Client::check_invited( std::string channel ) {
  try {
    for (std::vector<std::string>::iterator it = _invited.begin(); it != _invited.end(); it++) {
      if ((*it) == channel)
        return 1;
    }
  }
  catch (const std::exception &e) {
    printf("\n\n\n\n\t\t\t\tWIR HABEN IHN2\n");
  }
  return 0;
}


std::string Client::get_buf( void ) {
  return _buf;
}

std::string Client::get_user( void ) {
  return _user;
}

std::string Client::get_host( void ) {
  return _host;
}

std::string Client::get_nick( void ) {
  return _nick;
}

std::string Client::get_prefix( void ) {
  std::string ret;

  // printf("NICK: %s\n", _nick.c_str());
  // printf("USER: %s\n", _user.c_str());
  // printf("HOST: %s\n", _host.c_str());
  ret = ":" + _nick + "!~" + _user + "@" + _host;
  return ret;
}

bool Client::get_regi_set( void ) const {
  return _regi_set;
}

bool Client::get_pass_set( void ) const {
  return _pass_set;
}

bool Client::get_user_set( void ) const {
  return _user_set;
}

bool Client::get_nick_set( void ) const {
  return _nick_set;
}

int Client::get_channel_count( void ) const {
  return _channel_count;
}

int Client::get_fd( void ) const {
  return _cli_fd;
}

void Client::set_invite( std::string channel ) {
  printf("SET INVITE FOR %s channel: %s\n", get_nick().c_str(), channel.c_str());
  _invited.push_back(channel);
}
