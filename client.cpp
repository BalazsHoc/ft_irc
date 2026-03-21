#include "client.hpp"

Client::Client ( int cli_fd ) : 
  _pass_set(false),
  _regi_set(false),
  _user_set(false),
  _nick_set(false),
  _channel_count(0),
  _channel_limit(20),
  _cli_fd(cli_fd) {
  _nick = "*";
  _channel_count = 0;
}

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

void Client::set_out_buf( std::string out ) {
  _out_buf += out + "\r\n";
}

std::string Client::get_out_buf( void ) {
  return _out_buf;
}

void Client::send_out ( void ) {
  int n = send(_cli_fd, _out_buf.c_str(), _out_buf.size(), MSG_NOSIGNAL);
  if (n > 0)
    _out_buf.erase(0, n);
}

void Client::set_in_buf( std::string buf ) {
  _in_buf.clear();
  _in_buf = buf;
}

void Client::append_to_buf( std::string buf ) {
  _in_buf += buf;
}

void Client::clear_in_buf( void ) {
  _in_buf.clear();
}

void Client::unset_channel( std::string channel ) {
  for (int i = 0; i < _channel_count; i++) {
    if (_channel[i] == channel) {
      _channel[i] = "";
      _channel_count--;
    }
  }
}

void Client::set_channel( std::string channel ) {
  if (_channel_count >= _channel_limit) // not really needed but extra safe
    return ;
  for (int i = 0; i < _channel_count; i++) {
    if (_channel[i] == channel)
      return ;
  }
  int i = 0;
  while (i < _channel_count) {
    if (_channel[i] == "") {
      break ;
    }
    i++;
  }
  _channel[i] = channel;
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
    // debugging
  }
  return 0;
}


std::string Client::get_in_buf( void ) {
  return _in_buf;
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

std::string *Client::get_channels( void ) {
  return _channel;
}

int Client::get_channel_count( void ) const {
  return _channel_count;
}

int Client::get_fd( void ) const {
  return _cli_fd;
}

void Client::set_invite( std::string channel ) {
  _invited.push_back(channel);
}
