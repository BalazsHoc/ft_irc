CXX = c++
CXXFLAGS = -g #-Wall -Wextra -Werror -std=c++98 -MMD

NAME = irc

SRCS = main.cpp client.cpp channel.cpp

OBJ = $(SRCS:.cpp=.o)
DEP = $(OBJ:.o=.d)

%.o : %.cpp
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

$(NAME): $(OBJ)
	@$(CXX) $(OBJ) -o $(NAME)

all: $(NAME)

clean:
	@rm -f $(OBJ)
	@rm -f $(DEP)

fclean: clean
	@rm -f $(NAME)

re: fclean all

-include $(DEP)

.PHONY: all clean fclean re
