
BINDIR = bin
SRCDIR = src
BIN    = $(BINDIR)/3d-demo
SRC    = $(SRCDIR)/*.cpp
CFLAGS = --std=c++11 \
		 -lGL -lGLU -lglut -lGLEW \
		 -Ilib \
		 $(shell pkg-config --cflags --libs libpng) \
		#  $(shell pkg-config --cflags --libs boost-1.66)
ASSETS = assets
# ASSETS = design/assets

.PHONY: all build run clean install

all: clean build run install

build:
	mkdir -p $(BINDIR)
	$(CXX) $(SRC) -o $(BIN) $(CFLAGS)
	cp -r $(ASSETS)/* $(BINDIR)/

run:
	cd $(BINDIR) && ./$(notdir $(BIN))

clean:
	rm -rf $(BINDIR)

install:
	sudo apt-get install libpng-dev
	sudo apt-get install libboost-dev
