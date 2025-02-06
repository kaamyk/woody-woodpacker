.SILENT:
	
NAME	=	woody_woodpacker	

CC		=	gcc
CC_FLAGS	=	-Wall -Wextra -Werror -g3

C_SRCDIR	=	src/c
C_SRCS	=	$(C_SRCDIR)/main.c	$(C_SRCDIR)/error.c
OBJDIR	=	.obj
OBJS	=	$(patsubst $(C_SRCDIR)/%.c, $(OBJDIR)/%.o, $(C_SRCS))
DEPDIR	=	.dep
DEPS	=	$(patsubst $(C_SRCDIR)/%.c, $(DEPDIR)/%.d, $(C_SRCS))

$(OBJDIR):
	mkdir -v $(OBJDIR)

$(DEPDIR):
	mkdir -v $(DEPDIR)

$(OBJDIR)/%.o:	$(C_SRCDIR)/%.c
	echo -n "Compiling " $< " ... "
	$(CC) $(CC_FLAGS) -c $< -o $@
	$(CC) $(CC_FLAGS) -MM $< | sed 's,\($*\)\.o[ :]*,$(DEPDIR)/\1.o $@ : ,g' > $(DEPDIR)/$*.d
	echo -e '\x1b[32m>>> OK <<<\x1b[37m'

parasite:
	echo -n "Creating parasite opcode file 'woody' ... "
	cd ./src/asm
	bash ./src/asm/generate_sh_code.sh
	cd ../../
	echo -e '\x1b[32m>>> OK <<<\x1b[37m'

all:		$(OBJDIR) $(DEPDIR) $(OBJS) $(PARASITE) $(NAME)

$(NAME):	 $(OBJDIR) $(DEPDIR) $(OBJS) parasite
	echo -e '\033[0;34mObjects compilation: \x1b[32m>>> OK <<<\x1b[37m'
	echo ""
	echo -n "Compiling " $(NAME) " ... "
	$(CC) $(CC_FLAGS) $(OBJS) -lm -o $(NAME)
	echo -e '\x1b[32m>>> OK <<<\x1b[37m'
	echo -en '\033[0;34mExecutable '
	echo -n  $(NAME)
	echo -e  ' compiled: \x1b[32m>>> OK <<<\x1b[37m'
	echo ""
	
clean:
	rm -rfv $(OBJDIR) $(DEPDIR)
	echo -e '\033[0;34mObject/Dependencies files removed: \x1b[32m>>> OK <<<\x1b[37m'
	echo ""
	rm -fv src/asm/parasite.o
	echo -e '\033[0;34mParsite object file removed: \x1b[32m>>> OK <<<\x1b[37m'
	echo ""

fclean:		clean
	rm -fv $(NAME) $(NAME).d
	echo -e '\033[0;34mExecutable file removed: \x1b[32m>>> OK <<<\x1b[37m'
	echo ""
	rm -fv src/asm/opcode 
	echo -e '\033[0;34mParasite 'opcode' file removed: \x1b[32m>>> OK <<<\x1b[37m'
	echo ""

-include $(DEPS)

re:		fclean all

.PHONY:	all clean fclean