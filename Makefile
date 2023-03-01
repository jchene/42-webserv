################################################################################
#									 CONFIG								   #
################################################################################

S_NAME		:= server
C_NAME		:= client
CC			:= gcc
FLAGS		:= -Wall -Wextra -Werror -Wpedantic -g
 
################################################################################
#								 PROGRAM'S SRCS							   #
################################################################################

S_SRCS		:= server.c
C_SRCS		:= client.c

S_OBJS		:=  ${S_SRCS:.c=.o}
C_OBJS		:=  ${C_SRCS:.c=.o}

S_HEADERS 	:=
C_HEADERS 	:=

.c.o:
			${CC} ${FLAGS} -c $< -o ${<:.c=.o}

################################################################################
#								  Makefile  objs							  #
################################################################################

CLR_RMV		:= \033[0m
RED			:= \033[1;31m
GREEN		:= \033[1;32m
YELLOW		:= \033[1;33m
BLUE		:= \033[1;34m
CYAN		:= \033[1;36m
RM			:= rm -f

all:		server client

server:		${S_OBJS} ${S_HEADERS} Makefile
			@ echo "$(GREEN)Compilation ${CLR_RMV}of ${YELLOW}$(S_NAME) ${CLR_RMV}..."
			${CC} ${FLAGS} -o ${S_NAME} ${S_OBJS}
			@ if [ ! -d "objs" ]; then mkdir objs; fi
			@ mv *.o objs/
			@ echo "$(GREEN)$(S_NAME) created ✔️${CLR_RMV}"

client:		${C_OBJS} ${C_HEADERS} Makefile
			@ echo "$(GREEN)Compilation ${CLR_RMV}of ${YELLOW}${C_NAME} ${CLR_RMV}..."
			${CC} ${FLAGS} -o ${C_NAME} ${C_OBJS}
			@ if [ ! -d "objs" ]; then mkdir objs; fi
			@ mv *.o objs/
			@ echo "$(GREEN)${C_NAME} created ✔️${CLR_RMV}"

clean:
			@ ${RM} *.o */*.o */*/*.o
			@ rmdir objs
			@ echo "$(RED)Deleting $(CYAN)$(S_NAME) $(CLR_RMV)objs ✔️"
			@ echo "$(RED)Deleting $(CYAN)$(C_NAME) $(CLR_RMV)objs ✔️"

fclean:		clean
			@ ${RM} ${S_NAME}
			@ echo "$(RED)Deleting $(CYAN)$(S_NAME) $(CLR_RMV)binary ✔️"
			@ ${RM} ${C_NAME}
			@ echo "$(RED)Deleting $(CYAN)$(C_NAME) $(CLR_RMV)binary ✔️"

re:			fclean all

.PHONY:		all clean fclean re server client