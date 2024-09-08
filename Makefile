NAME = main  # Output executable name
SRCS = $(wildcard *.c) utils/compress.c utils/filter.c  # All .c source files
INCL = $(wildcard *.h)  # All header files
OBJS = $(SRCS:.c=.o)    # Object files from source files
CC = gcc  # Compiler
FLAGS = -Wall -Wextra -Werror -g  # Compilation flags
LIBS = -ljpeg -lm  # Libraries to link against
RM = rm -rf  # Command to remove files

.PHONY: all clean fclean re

# Default target: Build the executable
all: $(NAME)

# Linking step: Link object files into an executable
$(NAME): $(OBJS)
	@$(CC) $(FLAGS) -o $(NAME) $(OBJS) $(LIBS)  # Link the object files and link with libjpeg

# Compile .c files into .o object files
%.o: %.c $(INCL)
	@$(CC) $(FLAGS) -c $< -o $@  # Compile .c into .o

# Clean up object files
clean:
	@$(RM) $(OBJS)

# Clean up object files and the executable
fclean: clean
	@$(RM) $(NAME)
	@$(RM) *.jpg  # Optional: Remove all .jpg files (if generated)

# Rebuild the project
re: fclean all
