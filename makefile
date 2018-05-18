
BINDIR = bin
SRCDIR = src
BIN    = $(BINDIR)/3d-demo
SRC    = $(SRCDIR)/*.cpp
CFLAGS = --std=c++11 \
		 -lGL -lGLU -lglut -lGLEW \
		 $(shell pkg-config --cflags --libs libpng) \
		#  $(shell pkg-config --cflags --libs boost-1.66)
ASSETS = assets
# ASSETS = design/assets

.PHONY: build run clean

build:
	mkdir -p $(BINDIR)
	$(CXX) $(SRC) -o $(BIN) $(CFLAGS)
	cp -r $(ASSETS)/* $(BINDIR)/

run:
	cd $(BINDIR) && ./$(notdir $(BIN))

clean:
	rm -rf $(BINDIR)
