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
DBUS_CPPFLAGS		!=  pkg-config --cflags dbus-1
DBUS_LIBS			!=  pkg-config --libs dbus-1
DBUS_TINY_CPPFLAGS	:=	-I$(PWD)/DBUS-Tiny
DBUS_TINY_LIBS		:=	-L$(PWD)/DBUS-Tiny -Wl,-rpath=$(CWD)/DBUS-Tiny -ldbus-tiny
#CURLPP_CPPFLAGS		!= 	pkg-config --cflags curlpp
CURLPP_CPPFLAGS	:= 	-isystem /usr/include/curlpp -DWITH_GZFILEOP
CURLPP_LIBS			!= 	pkg-config --libs curlpp
BOOST_LIBS			:=	-lboost_system -lboost_program_options -lboost_regex -lboost_thread -lboost_chrono -lboost_json

CPPFLAGS			:= -O3 $(DBUS_TINY_CPPFLAGS) $(DBUS_CPPFLAGS) $(CURLPP_CPPFLAGS)

LDFLAGS				:= -O3 $(BOOST_LIBS) $(DBUS_TINY_LIBS) $(DBUS_LIBS) $(CURLPP_LIBS) \
						-lssl -lcrypto -lpthread

OBJS				:= tadoif.o
EXEC				:= tadoif

.PRECIOUS:			*.h *.cpp *.i
.PHONY:				all

all:				$(EXEC)

clean:
				$(VECHO) "CLEAN"
				-$(Q) rm -f $(OBJS) $(EXEC) 2> /dev/null

# -fpermissive for curl headers:
#  /usr/include/curlpp/Options.hpp:281:74: error: invalid conversion from ‘int’ to ‘CURLoption’ [-fpermissive]
# typedef curlpp::OptionTrait<curl_closepolicy, CURLOPT_CLOSEPOLICY> ClosePolicy;
%.o:			%.cpp
				$(VECHO) "CPP $< -> $@"
				$(Q) $(CPP) @gcc-warnings $(CPPFLAGS) -fpermissive -c $< -o $@

$(EXEC):		$(OBJS) 
				$(VECHO) "LD $<-> $@"
				$(Q) $(CPP) @gcc-warnings $(LDFLAGS) $< -o $@
