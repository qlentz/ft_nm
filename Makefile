NAME 	= ft_nm

SRCS	= src/main.c src/elf_ident.c src/safety_utils.c src/readers.c \
		  src/ctx.c src/sections.c src/symbols.c src/formatting.c \
		  src/errors.c

OBJS	= $(SRCS:.c=.o)

DEPS	= $(OBJS:.o=.d)

LIBFT_DIR	= ./libft

LIBFT	= $(LIBFT_DIR)/libft.a

CC		= gcc

CFLAGS	= -Wall -Wextra -Werror -MMD -MP -I. -I $(LIBFT_DIR)

RM		= rm -f

.c.o:
			${CC} ${CFLAGS} -c $< -o ${<:.c=.o}

all:		$(NAME)

$(NAME):	$(OBJS) $(LIBFT)
			$(CC) $(CFLAGS) -o $@ $(OBJS) -L $(LIBFT_DIR) -lft

$(LIBFT):	FORCE
			@$(MAKE) -sC $(LIBFT_DIR)

clean:
			$(RM) $(OBJS) $(DEPS)
			@$(MAKE) -C $(LIBFT_DIR) clean

fclean:		clean
			@$(MAKE) -C $(LIBFT_DIR) fclean
			$(RM) $(NAME)

re:			fclean all

FORCE:

-include $(DEPS)

.PHONY: all clean fclean re FORCE
