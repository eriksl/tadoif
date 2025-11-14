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
#CURLPP_CPPFLAGS	!= 	pkg-config --cflags curlpp
CURLPP_CPPFLAGS		:= 	-isystem /usr/include/curlpp -DWITH_GZFILEOP
CURLPP_LIBS			!= 	pkg-config --libs curlpp
BOOST_LIBS			:=	-lboost_system -lboost_program_options -lboost_regex -lboost_thread -lboost_chrono -lboost_json

CPPFLAGS		:= -O3 $(DBUS_TINY_CPPFLAGS) $(DBUS_CPPFLAGS) $(CURLPP_CPPFLAGS)

LDFLAGS			:= -O3 -L. -ltadoif -Wl,-rpath=$(CWD) \
						$(BOOST_LIBS) $(DBUS_TINY_LIBS) $(DBUS_LIBS) $(CURLPP_LIBS) \
						-lssl -lcrypto -lpthread

HDRS			:= tadoif.h
LIBOBJS			:= tadoif-lib.o exception.o
LIB				:= libtadoif.so
EXECSRCS		:= tadoif.cpp
EXECOBJS		:= tadoif.o
EXEC			:= tadoif

.PRECIOUS:		*.h *.cpp *.i
.PHONY:			all

all:			$(LIB) $(EXEC)

clean:
				$(VECHO) "CLEAN"
				-$(Q) rm -rf $(LIBOBJS) $(LIB) $(EXECOBJS) $(EXEC) 2> /dev/null

exception.o:	$(HDRS)
$(LIBOBJS):		$(HDRS)
$(EXECOBJS):	$(HDRS)

# -fpermissive for curl headers:
#  /usr/include/curlpp/Options.hpp:281:74: error: invalid conversion from ‘int’ to ‘CURLoption’ [-fpermissive]
# typedef curlpp::OptionTrait<curl_closepolicy, CURLOPT_CLOSEPOLICY> ClosePolicy;
%.o:			%.cpp
				$(VECHO) "CPP[LIB] $< -> $@"
				$(Q) $(CPP) @gcc-warnings $(CPPFLAGS) -fpermissive -fPIC -c $< -o $@

$(LIB):			$(LIBOBJS)
				$(VECHO) "LD[LIB] $(LIBOBJS) -> $@"
				$(Q) $(CPP) @gcc-warnings $(LDFLAGS) $(LIBOBJS) -fPIC -shared -o $@

$(EXECOBJS):	$(EXECSRCS)
				$(VECHO) "CPP $< -> $@"
				$(Q) $(CPP) @gcc-warnings $(CPPFLAGS) -fpermissive -c $< -o $@

$(EXEC):		$(EXECOBJS) $(LIB)
				$(VECHO) "LD $(EXECOBJS) -> $@"
				$(Q) $(CPP) @gcc-warnings $(LDFLAGS) $(EXECOBJS) -o $@
