NAME 	= ft_nm

SRCS	= src/main.c src/elf_ident.c src/safety_utils.c src/readers.c \
		  src/ctx.c src/sections.c src/symbols.c

OBJS	= $(SRCS:.c=.o)

LIBFT	= ./libft

CC		= gcc

CFLAGS	= -Wall -Wextra -Werror -I. -I ./libft

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
