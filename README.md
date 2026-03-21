This project has been created as part of the 42 curriculum by cjuarez, bhocsak.

# ft_irc

## Description

**ft_irc** is a custom implementation of an Internet Relay Chat (IRC) server written in **C++98**, following the constraints and standards of the 42 curriculum.

The goal of this project is to build a functional IRC server that can communicate with an IRC client (in this case, **irssi**) using the IRC protocol. The server must handle multiple clients simultaneously, manage channels, and support basic IRC commands.

Through this project, we explore:
- Network programming (sockets, TCP/IP)
- Multiplexing (e.g., `epoll()`)
- Protocol implementation (IRC RFC basics)

## Instructions
- Clone the repository and compile the project using:`make`
- **Usage**:
  - server: ./ircserv <port> <password>
  - client:
    - irssi
    - /set nick `nickname`
    - /set user_name `user_name`
    - ./connect 127.0.0.1 `port` `password` 
- Commands:
  - `NICK`
  - `USER`
  - `JOIN`
  - `PRIVMSG`
  - `KICK`
  - `INVITE`
  - `MODE` `+/-i`, `+/-t`, `+/-k`, `+/-o`, `+/-l`

## Resources
- RFC 1459 - Internet Relay Chat Protocol
- RFC 2812 - IRC Client Protocol
- Linux man pages (socket, poll, etc.)
- irssi documentation
