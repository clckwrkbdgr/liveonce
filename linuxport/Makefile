TARGET = liveonce

OFILES = linuxport.o \
	 ../gfxengine.o \
	 ../gfx_curses.o \
	 ../gfx_sdl.o \
	 ../grammar.o \
	 ../item.o \
	 ../map.o \
	 ../mob.o \
	 ../msg.o \
	 ../panel.o \
	 ../rand.o \
	 ../speed.o \
	 ../text.o \
	 ../quest.o \
	 ../glbdef.o

.cpp.o:
	g++ -O3 `sdl-config --cflags` -c $< -o $@

all: $(TARGET)

$(TARGET): $(OFILES)
	g++ -O3 -o $(TARGET) $(OFILES) `sdl-config --libs` -lcurses

clean:
	rm -f *.o
	rm -f $(TARGET)

premake:
	cp ../text.txt .
	cp ../rooms/valley.map .
	cp ../rooms/piecelist.map .
	cp ../gfx/alphabet_12x16.bmp alphabet.bmp
	cd .. ; enummaker source.txt
