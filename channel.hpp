#pragma once


#include <map>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>

// FOR STRING TO INT
#include <string>
#include <cerrno>
#include <cstdlib>
#include <climits>
#include <ctime>





class Channel {

  public:

    Channel( void );

    ~Channel( );


    void              set_pass        ( std::string pass )          ;
    void              set_topic       ( std::string topic, std::string nick )         ;
    void              set_client      ( int cli_fd, std::string name )                ;
    void              set_op          ( int cli_fd, std::string name )                ;
    bool              set_limit       ( std::string limit )         ;
    void              drop_client     ( int cli_fd )                ;
    void              set_name        (std::string name);
    std::string       get_name        ( void )                ;

    void              set_invite_set  ( bool value )                ;
    void              set_topic_set   ( bool value )                ;
    void              unset_pass      ( void )                      ;
    void              unset_limit     ( void )                      ;

    int               check_op        ( int cli_fd )                ;
    int               check_client    ( int cli_fd )                ;

    std::string       get_pass        ( void )                const ;
    std::string       get_topic       ( void )                const ;
    std::string       get_setter_nick ( void )                const ;

    size_t            get_user_count  ( void )                const ;
    size_t            get_limit       ( void )                const ;

    time_t            get_topic_timestamp ( void )            const ;

    std::map<int, std::string>  get_clients     ( void )      const ;
    std::map<int, std::string>  get_ops         ( void )      const ;

    bool              get_pass_set    ( void )                      ;
    bool              get_topic_set   ( void )                      ;
    bool              get_invite_set  ( void )                      ;

  private:

    bool          _pass_set = 0;
    bool          _invite_set = 0;
    bool          _topic_set = 0;
    

    std::string   _topic;

    std::string   _setter_nick; // who set the topic
    std::string   _pass;
    std::string   _channel_name; // not needed just for debugging left

    size_t        _limit = 200;
    size_t        _user_count = 0;

    time_t        _timestamp; // topic

    std::map<int, std::string> _ops; // who are the operators stored as fd, string

    std::map<int, std::string> _clients; // who is in

};
