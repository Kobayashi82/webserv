# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/02/20 17:51:39 by vzurera-          #+#    #+#              #
#    Updated: 2024/10/03 13:08:32 by vzurera-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# ───────────────────────────────────────────────────────────── #
# ─────────────────────── CONFIGURATION ─────────────────────── #
# ───────────────────────────────────────────────────────────── #

# ──────────── #
# ── COLORS ── #
# ──────────── #

RED     			= \033[0;31m
GREEN   			= \033[0;32m
YELLOW  			= \033[0;33m
BLUE    			= \033[0;34m
MAGENTA 			= \033[0;35m
CYAN    			= \033[0;36m
WHITE   			= \033[0;37m
NC    				= \033[0m
INV_RED  			= \033[7;31m
INV_GREEN	  		= \033[7;32m
INV_YELLOW  		= \033[7;33m
INV_BLUE  			= \033[7;34m
INV_MAGENTA			= \033[7;35m
INV_CYAN			= \033[7;36m
INV_WHITE			= \033[7;37m
BG_CYAN				= \033[40m
FG_YELLOW			= \033[89m
COUNTER 			= 0

# ─────────── #
# ── FLAGS ── #
# ─────────── #

CC					= clang++
FLAGS				= -Wall -Wextra -Werror -std=c++98 -g # -fsanitize=thread

# ───────────────── #
# ── DIRECTORIES ── #
# ───────────────── #

SRC_DIR				= ./src/
INC_DIR				= -I./inc -I./inc/Thread -I./inc/Log -I./inc/Display -I./inc/Utils -I./inc/Settings -I./inc/Network -I./inc/Protocol
OBJ_DIR				= ./build/obj/

# ────────── #
# ── NAME ── #
# ────────── #

NAME	=	webserv
SRCS	=	Thread/Thread.cpp																											\
			Log/Timer.cpp Log/Log.cpp																									\
			Display/Display.cpp Display/Monitor.cpp																						\
			Utils/String.cpp Utils/Network.cpp Utils/Files.cpp																			\
			Settings/Settings.cpp Settings/VServer.cpp Settings/Location.cpp Settings/Method.cpp Settings/Parser.cpp Settings/Codes.cpp	\
			Network/Socket.cpp Network/Event.cpp Network/Epoll.cpp Network/Client.cpp													\
			Protocol/Cache.cpp Protocol/Communication.cpp Protocol/Request.cpp Protocol/Response.cpp Protocol/Security.cpp				\
			main.cpp

# ────────────────────────────────────────────────────────── #
# ───────────────────────── NORMAL ───────────────────────── #
# ────────────────────────────────────────────────────────── #

all: $(NAME)

OBJS		= $(SRCS:%.cpp=$(OBJ_DIR)%.o)
DEPS		= $(OBJS:.o=.d)
-include $(DEPS)

#	Compile if the executable does not exist or there are changes in the .cpp files.
#	This avoids re-linking and shows a more appropriate message.
#	If you run "make -n", it should not show any compilation commands if it is already compiled.
#	But it will show the "make -s" calls. This is to check if compilation is necessary.
$(NAME): normal
	@if [ ! -f bin/$(NAME) ]; then \
        $(MAKE) -s Compile; \
	elif $(MAKE) -s -q $(OBJS); then \
		$(MAKE) -s UpToDate; \
	else \
		$(MAKE) -s Compile; \
	fi

#	Show "Up to date" message
UpToDate:
	@printf "\r%50s\r\t$(CYAN)Up to date    $(GREEN)✓ $(YELLOW)$(NAME)$(NC)\n"; \
	$(MAKE) -s progress; printf "\n"; \
	$(MAKE) -s show_cursor; \

#	Compile program
Compile: $(OBJS)
	@printf "\r%50s\r\t$(CYAN)Compiling... $(YELLOW)$(NAME)$(NC)"
	@mkdir -p bin
	@$(CC) $(FLAGS) $(INC_DIR) $(OBJS) -o bin/$(NAME);
	@printf "\r%50s\r\t$(CYAN)Compiled    $(GREEN)✓ $(YELLOW)$(NAME)$(NC)\n"
#	Create launcher for root folder (Temp)
	@printf 'chmod +x bin/cgi-bin/*\ncd bin && ./webserv "$$@"\n' > webserv
	@chmod +x webserv
#	Progress line
	@$(MAKE) -s progress; printf "\n"
#	Restore cursor
	@$(MAKE) -s show_cursor

# ───────────── #
# ── OBJECTS ── #
# ───────────── #

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
#	Create folder
	@mkdir -p $(@D)
#	Compile objects
	@filename=$$(basename $<); filename=$${filename%.*}; \
	BAR=$$(printf "/ ─ \\ |" | cut -d" " -f$$(($(COUNTER) % 4 + 1))); \
	printf "\r%50s\r\t$(CYAN)Compiling... $(GREEN)$$BAR  $(YELLOW)$$filename$(NC)"; \
	$(eval COUNTER=$(shell echo $$(($(COUNTER)+1))))
	@$(CC) $(FLAGS) $(INC_DIR) -MMD -o $@ -c $<

# ───────────────── #
# ── EXTRA RULES ── #
# ───────────────── #

normal:
	@if [ ! -d "$(SRC_DIR)" ]; then \
		printf "\n\t$(CYAN)source files doesn't exist\n\n$(NC)"; \
		rm -f .is_re; \
		exit 1; \
	fi
#	Hide cursor
	@$(MAKE) -s hide_cursor
#	Title
	@if [ ! -f .is_re ]; then \
		clear; $(MAKE) -s title; \
	else \
		printf "\t$(WHITE)─────────────────────────\n$(NC)"; \
		printf "\n\t─────────────────────────$(NC)\033[1A\r"; \
	fi; rm -f .is_re

# ───────────────────────────────────────────────────────────── #
# ────────────────────────── RE-MAKE ────────────────────────── #
# ───────────────────────────────────────────────────────────── #

re:
	@rm -f .is_re
	@if [ ! -d "$(SRC_DIR)" ]; then \
		printf "\n\t$(CYAN)source files doesn't exist\n\n$(NC)"; \
		exit 1; \
	fi
#	Hide cursor
	@$(MAKE) -s hide_cursor
	@$(MAKE) -s fclean
#	Create files
	@touch .is_re
	@printf "\033[1A\033[1A\r"
#	Execute $(NAME)
	@$(MAKE) -s $(NAME)

# ───────────────────────────────────────────────────────────── #
# ─────────────────────────── CLEAN ─────────────────────────── #
# ───────────────────────────────────────────────────────────── #

clean:
	@rm -f .is_re
#	Hide cursor
	@$(MAKE) -s hide_cursor
#	Title
	@clear
	@$(MAKE) -s title
#	Delete objects
	@$(MAKE) -s delete_objects
	@printf "\r%50s\r\t$(CYAN)Deleted     $(GREEN)✓ $(YELLOW)objects$(NC)\n"
#	Progress line
	@$(MAKE) -s progress; printf "\n"
#	Restore cursor
	@$(MAKE) -s show_cursor

# ───────────────────────────────────────────────────────────── #
# ────────────────────────── F-CLEAN ────────────────────────── #
# ───────────────────────────────────────────────────────────── #

fclean:
	@rm -f .is_re webserv
#	Hide cursor
	@$(MAKE) -s hide_cursor
#	Title
	@clear
	@$(MAKE) -s title
#	Delete objects
	@$(MAKE) -s delete_objects
#	Delete $(NAME)
	@if [ -f bin/$(NAME) ]; then \
		printf "\t$(CYAN)Deleting... $(YELLOW)$(NAME)$(NC)"; \
		rm -f bin/$(NAME); \
	fi
	@printf "\r%50s\r\t$(CYAN)Deleted     $(GREEN)✓ $(YELLOW)$(NAME)$(NC)\n"
	@$(MAKE) -s progress
	@printf "\n"
#	Delete folder and files
	@-rm -rf ./build >/dev/null 2>&1 || true
#	Restore cursor
	@$(MAKE) -s show_cursor

# ───────────────────────────────────────────────────────────── #
# ───────────────────────── FUNCTIONS ───────────────────────── #
# ───────────────────────────────────────────────────────────── #

# ─────────── #
# ── TITLE ── #
# ─────────── #

title:
	@printf "\n$(NC)\t$(INV_CYAN)          $(shell echo $(NAME) | tr a-z A-Z | tr '_' ' ')        $(NC)\n"
	@printf "\t$(WHITE)─────────────────────────\n$(NC)";
	@printf "\n\t─────────────────────────$(NC)\033[1A\r"

# ──────────── #
# ── CURSOR ── #
# ──────────── #

hide_cursor:
	@printf "\e[?25l"
 
show_cursor:
	@printf "\e[?25h"

# ──────────────────── #
# ── DELETE OBJECTS ── #
# ──────────────────── #

delete_objects:
	@printf "\n\t$(WHITE)─────────────────────────$(NC)\033[1A\r"
	@if [ -n "$(shell find $(OBJ_DIR) -type f -name '*.o' 2>/dev/null)" ]; then \
		COUNTER=0; \
		find $(OBJ_DIR) -type f -name '*.o' | while read -r file; do \
			BAR=$$(printf "/ ─ \\ |" | cut -d" " -f$$((COUNTER % 4 + 1))); \
			filename=$$(basename $$file); \
			for src in $(SRCS); do \
				srcname=$$(basename $$src .cpp); \
				if [ $$filename = $$srcname.o ]; then \
					rm -f $$file $$(dirname $$file)/$$srcname.d; \
					filename=$${filename%.*}; \
					printf "\r%50s\r\t$(CYAN)Deleting... $(GREEN)$$BAR $(YELLOW)$$filename$(NC)"; sleep 0.05; \
					COUNTER=$$((COUNTER+1)); \
					break; \
				fi; \
			done; \
		done; \
	fi; printf "\r%50s\r"
	@-find $(OBJ_DIR) -type d -empty -delete >/dev/null 2>&1 || true

# ─────────────────── #
# ── PROGRESS LINE ── #
# ─────────────────── #

progress:
	@total=25; printf "\r\t"; for i in $$(seq 1 $$total); do printf "$(RED)─"; sleep 0.01; done; printf "$(NC)"
	@total=25; printf "\r\t"; for i in $$(seq 1 $$total); do printf "─"; sleep 0.01; done; printf "\n$(NC)"

# ─────────── #
# ── PHONY ── #
# ─────────── #


wipe:
	@rm -rf build; rm -f .is_re; rm -f $(NAME); rm -f bin/$(NAME);

.PHONY: all clean fclean re normal delete_objects title hide_cursor show_cursor progress Compile UpToDate wipe