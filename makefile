
BINDIR = bin
SRCDIR = src
BIN    = $(BINDIR)/3d-demo
SRC    = $(SRCDIR)/*.cpp
CFLAGS = --std=c++11 \
		 -lGL -lGLU -lglut -lGLEW \
		 $(shell pkg-config --cflags --libs boost-1.66)

.PHONY: build run clean

build:
	mkdir -p $(BINDIR)
	$(CXX) $(SRC) -o $(BIN) $(CFLAGS)

run:
	cd $(BINDIR) && ./$(notdir $(BIN))

clean:
	rm -f $(BIN)
