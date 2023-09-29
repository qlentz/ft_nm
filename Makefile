NAME 	= ft_nm

SRCS	= src/main.c

OBJS	= $(SRCS:.c=.o)

LIBFT	= ./libft

CC		= gcc

CFLAGS	= -Wall -Wextra -Werror -I.

RM		= rm -f

.c.o:
			${CC} ${CFLAGS} -c $< -o ${<:.c=.o}

all:		lib $(NAME)

$(NAME):	$(OBJS)
			$(CC) $(CFLAGS) -L $(LIBFT) -o $@ $^ -lft

lib:
			@make -sC $(LIBFT)

clean:		
			$(RM) $(OBJS)
			@make -C $(LIBFT) clean


fclean:		clean
			@make -C $(LIBFT) clean
			$(RM) $(NAME)

re:			fclean $(NAME)

.PHONY: all clean fclean re
