###############################################################################
#    file: makefile for ray.cc
# created: 1-17-2005
#  author: J. Edward Swan II
###############################################################################
# Source file information

MAIN	=	mp3liblg

SRCS    =	main.cpp
OBJS	=	$(SRCS:.cpp=.o)

###############################################################################
# General compilation settings.  
DESTDIR		= .
INCLUDE 	=
DEFINES 	=
LDFLAGS		=
LIBDIR		= 
LIBS		= 

# For an optimized version, put this definition last.  Note that according to
# the gcc/g++ web page, inlined functions are not actually inlined unless the
# "-O3" flag is specified.  "-DNDEBUG" removes assertions.
#OPTIM		= -DNDEBUG
#-O2
# For a debug version, put this definition last.  "-Wall" prints copious,
# lint-type error messages.  "-g" produces a symbol table suitable for a
# debugger such as GDB.
#OPTIM		= -Wall -g
OPTIM = -O3

# Collect all the compilation settings
CPPFLAGS	= $(OPTIM) $(INCLUDE) $(DEFINES)

###############################################################################
# Programs to run
CC		= g++
RM		= /bin/rm -f
MAKE		= /usr/bin/make
MKDEPEND	= makedepend $(DEFINES) $(INCLUDE) $(SRCS)
LINK		= $(CC) $(LDFLAGS) -o $(DESTDIR)/$@ $(OBJS) $(LIBDIR) $(LIBS)

###############################################################################
# Explicit rules
.PRECIOUS: $(SRCS)

$(MAIN): $(OBJS)
	$(LINK)

all:
	$(MAKE) clean
	$(MAKE) $(MAIN)

clean:
	$(RM) *.o $(MAIN)

###############################################################################
# Dependency info
# Do not put stuff below this line; makedepend will clobber it!
# DO NOT DELETE THIS LINE -- make depend depends on it.

main.o: DirScanner.h DirScannerC.h
# DirScanner.o: DirScanner.h
