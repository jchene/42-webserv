NAME				=	webserv
SRCSDIR				=	srcs
OBJSDIR				=	objs
INCLUDESDIR			=	includes
OBJS_SUBDIR			=	$(shell find $(SRCSDIR) -type d | grep '/' | sed 's/srcs/objs/g')
CFLAGS				=	-Wall -Wextra -Werror -std=c++98 -g
CC					=	c++
RM					=	rm -rf

SRCS				=	$(shell find $(SRCSDIR) -type f -name "*.cpp")
SRCS_COUNT			=	$(shell find $(SRCSDIR) -type f -name "*.cpp" | wc -l)
OBJS				=	$(subst $(SRCSDIR),$(OBJSDIR),$(SRCS:.cpp=.o))
HEADERS				=	$(shell find $(INCLUDESDIR) -type f -name "*.hpp")
HEADERS_COUNT		=	$(shell find $(INCLUDESDIR) -type f -name "*.hpp" | wc -l)

INDEX				=	0
BUILD_SIZE			=	$(SRCS_COUNT)

_STOP				=	\e[0m
_GREEN				=	\e[92m
CLEAR				=	'	                                                     \r'
FULL				=	-->[$(_GREEN)===================================$(_STOP)]<--[ $(_GREEN)100%$(_STOP) ]

define update_bar	=
	BAR				=	-->[$(_GREEN)$(shell printf "%0.s=" $$(seq 0 $(shell echo "$(INDEX) * 33 / $(BUILD_SIZE)" | bc)))$(shell printf "%0.s " $$(seq $(shell echo "$(INDEX) * 33 / $(BUILD_SIZE)" | bc) 33))$(_STOP)]<--[ $(_GREEN)$(shell echo "$(shell echo "$(INDEX) * 33 / $(BUILD_SIZE)" | bc) * 3" | bc)%$(_STOP) ]
endef

#--------------------------------------------------------->>

all					:	$(NAME)

ifeq ($(HEADERS_COUNT), 1)
ifeq ($(SRCS_COUNT), 12)
$(NAME)				:	$(OBJSDIR) $(OBJS_SUBDIR) $(OBJS)
						@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
						@echo -ne $(CLEAR)
						@echo -e '	$(NAME) $(_GREEN)created$(_STOP).'
						@echo -e '	$(FULL)'
else
$(NAME)				:
						@echo "Srcs corrupted, aborting"
endif
else
$(NAME)				:
						@echo "Srcs corrupted, aborting"
endif

$(OBJSDIR)			:
						@mkdir $(OBJSDIR)

$(OBJS_SUBDIR)		:
						@mkdir $(OBJS_SUBDIR)

$(OBJSDIR)/%.o		:	$(SRCSDIR)/%.cpp $(HEADERS)
						@$(CC) $(CFLAGS) -c $< -o $(<:.cpp=.o)
						@mv $(SRCSDIR)/*/*.o $@
						@$(eval INDEX=$(shell echo $$(($(INDEX)+1))))
						@$(eval $(call update_bar))
						@echo -ne $(CLEAR)
						@echo -e '	$@ $(_GREEN)created$(_STOP).'
						@echo -ne '	$(BAR)\r'

clean				:
						@$(RM) $(OBJSDIR)
						@echo -ne $(CLEAR)
						@echo -e '	objs directory $(_GREEN)removed$(_STOP).'

fclean				:	clean
						@$(RM) $(NAME)
						@echo -ne $(CLEAR)
						@echo -e '	$(NAME) $(_GREEN)removed$(_STOP).'

re					:	fclean all

.PHONY				:	all clean fclean re
