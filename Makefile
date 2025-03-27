MAKEFLAGS += --no-builtin-rules

V ?= $(VERBOSE)
ifeq ($(V),1)
	Q :=
	VECHO := @true
	MAKEMINS :=
else
	Q := @
	VECHO := @echo
	MAKEMINS := -s
endif

CPP				:=	g++

CWD					!=	/bin/pwd
DBUS_CFLAGS			!=  pkg-config --cflags dbus-1
DBUS_LIBS			!=  pkg-config --libs dbus-1
DBUS_TINY_CFLAGS	:=	-I$(PWD)/DBUS-Tiny
DBUS_TINY_LIBS		:=	-L$(PWD)/DBUS-Tiny -Wl,-rpath=$(CWD)/DBUS-Tiny -ldbus-tiny
CURLPP_CFLAGS		!= 	pkg-config --cflags curlpp
CURLPP_LIBS			!= 	pkg-config --libs curlpp

CPPFLAGS		:= -O3 -fPIC -Wall -Wextra -Werror -Wframe-larger-than=65536 -Wno-error=ignored-qualifiers $(MAGICK_CFLAGS) $(DBUS_TINY_CFLAGS) $(DBUS_CFLAGS) \
					-lssl -lcrypto -lpthread -lboost_system -lboost_program_options -lboost_regex -lboost_thread -lboost_chrono -lboost_json \
							$(MAGICK_LIBS) $(DBUS_TINY_LIBS) $(DBUS_LIBS) $(CURLPP_LIBS) \

LIBOBJS			:= tadoif.o exception.o
LIB				:= libtadoif.so
EXEC			:= tadoif-server
EXECOBJS		:= $(EXEC).o
HDRS			:= tadoif.h

.PRECIOUS:		*.h *.cpp *.i
.PHONY:			all

all:			$(LIB) $(EXEC)

clean:
				$(VECHO) "CLEAN"
				-$(Q) rm -rf $(LIBOBJS) $(LIB) $(EXECOBJS) $(EXEC) 2> /dev/null

exception.o:	$(HDRS)
$(LIBOBJS):		$(HDRS)
$(EXECOBJS):	$(HDRS)

%.o:			%.cpp
				$(VECHO) "CPP $< -> $@"
				$(Q) $(CPP) @gcc-warnings $(CPPFLAGS) -c $< -o $@

$(LIB):			$(LIBOBJS)
				$(VECHO) "LD $(LIBOBJS) -> $@"
				$(Q) $(CPP) @gcc-warnings $(CPPFLAGS) $(LIBOBJS) -shared -o $@

$(EXEC):		$(EXECOBJS) $(LIB)
				$(VECHO) "LD $(EXECOBJS) -> $@"
				$(Q) $(CPP) @gcc-warnings $(CPPFLAGS) $(EXECOBJS) -L. -ltadoif -Wl,-rpath=$(CWD) -o $@
