
BINDIR = bin
SRCDIR = src
BIN    = $(BINDIR)/prpr
SRC    = $(SRCDIR)/*.cpp
CFLAGS = -lGL -lGLU -lglut -lGLEW --std=c++11

.PHONY: build run clean

build:
	mkdir -p $(BINDIR)
	$(CXX) $(SRC) -o $(BIN) $(CFLAGS)

run:
	cd $(BINDIR) && ./$(notdir $(BIN))

clean:
	rm $(BIN)
