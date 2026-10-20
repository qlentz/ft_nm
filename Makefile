NAME 	= ft_nm

SRC_DIR	= src

OBJ_DIR	= obj

SRCS	= main.c errors.c file.c readers.c safety_utils.c \
		  elf_ident.c elf_utils.c elf_header.c section_table.c \
		  sections.c symtab.c symbol_read.c symbol_type.c \
		  sort.c render.c formatting.c

OBJS	= $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))

DEPS	= $(OBJS:.o=.d)

LIBFT_DIR	= ./libft

LIBFT	= $(LIBFT_DIR)/libft.a

CC		= gcc

CFLAGS	= -Wall -Wextra -Werror -MMD -MP -I. -I $(LIBFT_DIR)

RM		= rm -f

all:		$(NAME)

$(NAME):	$(OBJS) $(LIBFT)
			$(CC) $(CFLAGS) -o $@ $(OBJS) -L $(LIBFT_DIR) -lft

$(LIBFT):	FORCE
			@$(MAKE) -sC $(LIBFT_DIR)

$(OBJ_DIR)/%.o:	$(SRC_DIR)/%.c | $(OBJ_DIR)
			$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
			mkdir -p $(OBJ_DIR)

clean:
			$(RM) -r $(OBJ_DIR)
			@$(MAKE) -C $(LIBFT_DIR) clean

fclean:		clean
			@$(MAKE) -C $(LIBFT_DIR) fclean
			$(RM) $(NAME)

re:			fclean all

FORCE:

-include $(DEPS)

.PHONY: all clean fclean re FORCE
