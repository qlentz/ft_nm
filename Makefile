NAME 	= ft_nm

SRCS	= src/main.c src/errors.c src/file.c src/readers.c src/safety_utils.c \
		  src/elf_ident.c src/elf_utils.c src/elf_header.c src/section_table.c \
		  src/sections.c src/symtab.c src/symbol_read.c src/symbol_type.c \
		  src/sort.c src/render.c src/formatting.c

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
