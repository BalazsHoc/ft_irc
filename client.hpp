#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include "channel.hpp"

class Client {

  public:

    Client ( int cli_fd );
    // Client ( Client & orig );
    // Client & operator = ( Client & orig );
    ~Client ( ) ;


    void set_fd( int cli_fd );
    void set_in_buf( std::string buf );
    void append_to_buf( std::string buf );
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

    void set_out_buf( std::string out );
    void send_out( void );

    void set_invite( std::string channel );
    void unset_invite( std::string channel );
    bool check_invited( std::string channel );

    bool get_regi_set( void ) const;
    bool get_pass_set( void ) const;
    bool get_user_set( void ) const;
    bool get_nick_set( void ) const;

    int get_channel_count( void ) const;
    std::string *get_channels( void ) ;
    void  unset_channel( std::string ) ;

    int get_fd( void ) const;

    void clear_in_buf( void ) ;


    std::string get_in_buf( void );
    std::string get_out_buf( void );
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

    std::string                 _channel[20];
    std::vector<std::string>    _invited;

    std::string _in_buf;
    std::string _out_buf;

};
