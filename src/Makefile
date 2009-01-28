
CC=gcc

CFLAGS=-c -Wall 

LDFLAGS= -all-static

LIBS=-lpdcurses -lregex
#c:/MinGW/lib/libgnurx.dll.a

SOURCES=background.c bookmarks.c color_scheme.c commands.c config.c\
 file_info.c filelist.c fileops.c filetype.c keys.c menus.c registers.c\
 rline.c search.c signals.c sort.c  ui.c utils.c vifm.c visual.c

OBJECTS=$(SOURCES:.c=.o)

EXECUTABLE=vifm.exe

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS) $(LIBS) 
.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o
