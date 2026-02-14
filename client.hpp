#pragma once

#include <string>
#include "channel.hpp"

class Client {

  public:

    Client ( void );
    // Client ( Client & orig );
    // Client & operator = ( Client & orig );
    ~Client ( ) ;

    void set_buf( char *buf );
    void set_user( std::string name );

    void set_nick( std::string nick );
    void set_real( std::string realname );
    void set_serv( std::string servername );
    void set_host( unsigned long ip_addr );
    void set_channel( std::string channel );

    void set_pass_set( bool value );
    void set_user_set( bool value ) ;
    void set_nick_set( bool value );
    void set_regi_set( bool value );

    bool get_regi_set( void ) const;
    bool get_pass_set( void ) const;
    bool get_user_set( void ) const;
    bool get_nick_set( void ) const;

    int get_channel_count( void ) const;
    int get_fd( void ) const;

    void clear_buf( void ) ;


    char *get_buf( void );
    std::string get_user( void );
    std::string get_nick( void );
    std::string get_host( void );

    std::string get_prefix( void ); // 


  private:

    bool  _pass_set = 0; // has correct password ?
    bool  _regi_set = 0; // is he already registered?

    bool  _user_set = 0; // user / nick / passw
    bool  _nick_set = 0;

    int   _channel_count = 0;
    int   _channel_limit = 20;
    int   _cli_fd;

    std::string _nick;
    std::string _user;
    std::string _host;
    std::string _realname;

    std::string    _channel[20];

    char *_buf;

};
