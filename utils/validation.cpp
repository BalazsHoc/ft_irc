#include "../main.hpp"

int is_alpha( char c ) {
  if (c >= 97 && c <= 122)
    return 1;
  if (c >= 65 && c <= 90)
    return 1;
  return 0;
}

int is_num( char c ) {
  if ( c >= 48 && c <= 57 )
    return 1;
  return 0;
}

int is_special ( char c ) {
  if (c == 45 || (c >= 91 && c <= 96)) {
    if (c == 95)
      return 0;
    return 1;
  }
  return 0;
}

int is_valid_nick( std::string nick ) {
  if (nick.empty() || !is_alpha(nick.at(0)))
    return 0;
  for (int i = 1; i < (int)nick.size(); i++) {
    if (!is_alpha(nick.at(i)) && !is_special(nick.at(i)) && !is_num(nick.at(i)))
      return 0;
  }
  return 1;
}

int is_valid_char( std::string user ) {
  for (int i = 1; i < (int)user.size(); i++) {
    if (user.at(i) == '\r' || user.at(i) == '\n' || user.at(i) == '\0' || user.at(i) == ' ')
      return 0;
  }
  return 1;
}

int valid_chars( std::string str ) {

  for (int i = 0; i < (int)str.size(); i++) {
    if (str.at(i) <= 32 || str.at(i) >= 127 || str.at(i) == ',' || str.at(i) == ':')
      return 0;
  }
  return 1;
}

int is_valid_cmnd( std::string str ) {
  if (str == "INVITE" || str == "KICK" || str == "MODE" || str == "JOIN"
      || str == "PASS" || str == "USER" || str == "NICK" || str == "PRIVMSG" || str == "TOPIC" || str == "PING" || str == "CAP" || str == "WHO")
    return 1;
  return 0;
}
