TARGET = enummaker

OFILES = enummaker.o \
	 StdAfx.o

.cpp.o:
	g++ -O3 `sdl-config --cflags` -c $< -o $@

all: $(TARGET)

$(TARGET): $(OFILES)
	g++ -O3 -o $(TARGET) $(OFILES) `sdl-config --libs`

clean:
	rm -f *.o
	rm -f $(TARGET)
