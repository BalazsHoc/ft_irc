#include "channel.hpp"
#include <stdexcept>
#include <string.h>


Channel::Channel ( void ) {

}

// Channel::Channel ( Channel & orig ) {}
// Channel::Channel & operator = ( Channel & orig ) {}

Channel::~Channel ( ) {

}


std::string Channel::get_pass( void ) const {
  return _pass;
}

std::string Channel::get_topic( void ) const {
  return _topic;
}


size_t Channel::get_limit( void ) const {
  return _limit;
}

size_t Channel::get_user_count( void ) const {
  return _user_count;
}


void  Channel::set_name(std::string name) {
  _channel_name = name;
}
std::string Channel::get_name( void ) {
  return _channel_name;
}

std::map<int, std::string> Channel::get_clients( void  ) const {
  return _clients;
}

std::map<int, std::string> Channel::get_ops( void  ) const {
  return _ops;
}

std::string Channel::get_setter_nick( void ) const {
  return _setter_nick;
}

time_t Channel::get_topic_timestamp( void ) const {
  return _timestamp;
}

void Channel::set_pass( std::string pass ) {
  _pass = pass;
  _pass_set = true;
}

void Channel::unset_pass( void ) {
  _pass = nullptr;
  _pass_set = false;
}

void Channel::unset_limit( void ) {
  _limit = 200;
}

void Channel::set_topic( std::string topic, std::string nick ) {
  _topic = topic;
  _topic_set = true;
  _timestamp = std::time(NULL);
  _setter_nick = nick;
}


bool str_to_int(const std::string& input, int& output)
{
  if (input.empty())
    return false;

  char* endptr = NULL;

  errno = 0; // reset before call
  long value = std::strtol(input.c_str(), &endptr, 10);

  // 1. No digits were found
  if (endptr == input.c_str())
    return false;

    // 2. Extra characters after number
  if (*endptr != '\0')
    return false;

    // 3. Overflow / underflow detected by strtol
  if ((errno == ERANGE && (value == LONG_MAX || value == LONG_MIN)))
    return false;

    // 4. Out of int range
  if (value > INT_MAX || value < INT_MIN)
    return false;

  output = static_cast<int>(value);
  return true;
}

bool Channel::set_limit( std::string limit ) {
  int res;
  if (str_to_int(limit, res) && res > 0 && res <= 200) {
    _limit = res;
    return 1;
  }
  else
    return 0;
}

void Channel::set_client( int cli_fd, std::string name ) {
  _clients[cli_fd] = name;
  _user_count++;
}

void Channel::drop_client( int cli_fd ) {
  for ( std::map<int, std::string>::iterator it = _clients.begin(); it != _clients.end(); it++ ) {
    if (it->first == cli_fd) {
      _clients.erase(it);
      return ;
    }
  }
}

void Channel::set_invite_set( bool value ) {
  _invite_set = value;
}


void Channel::set_topic_set( bool value ) {
  _topic_set = value;
}

void Channel::set_op( int cli_fd, std::string name) {
  _ops[cli_fd] = name;
}

bool Channel::get_topic_set( void ) {
  return _topic_set;
}

bool Channel::get_invite_set( void ) {
  return _invite_set;
}

bool Channel::get_pass_set( void ) {
  return _pass_set;
}

int Channel::check_op( int cli_fd ) {
  // OR USE FIND WHATEVER.
  for (std::map<int, std::string>::iterator it = _ops.begin(); it != _ops.end(); it++) {
    if (cli_fd == it->first)
      return 1;
  }
  return 0;
}

int Channel::check_client( int cli_fd ) {
  // OR USE FIND WHATEVER.
  if (_clients.empty())
    return 0;
  // printf("WE CHECK CLIENT AT: %d\n", cli_fd);
  try {
    for (std::map<int, std::string>::iterator it = _clients.begin(); it != _clients.end(); it++) {
      if (cli_fd == it->first)
        return 1;
    }
  } catch (const std::out_of_range &e) {
    printf("\n\n\n\n\t\t\t\tWIR HABEN IHN1\n");
  }
  return 0;
}



