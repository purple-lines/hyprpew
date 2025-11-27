CXX = g++
CXXFLAGS = -shared -fPIC -std=c++2b -O2 -g

ifeq ($(CXX),g++)
    EXTRA_FLAGS = --no-gnu-unique
else
    EXTRA_FLAGS =
endif

PKG_CONFIG_DEPS = pixman-1 libdrm hyprland pangocairo libinput libudev wayland-server xkbcommon
PKG_CONFIG_FLAGS = $(shell pkg-config --cflags $(PKG_CONFIG_DEPS))

SRCS = main.cpp CurtainPassElement.cpp
OBJS = $(SRCS:.cpp=.o)

TARGET = hyprpew.so

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(SRCS) -o $(TARGET) $(PKG_CONFIG_FLAGS)

clean:
	rm -f $(TARGET) $(OBJS)

install: $(TARGET)
	mkdir -p ~/.local/share/hyprload/plugins
	cp $(TARGET) ~/.local/share/hyprload/plugins/

uninstall:
	rm -f ~/.local/share/hyprload/plugins/$(TARGET)

.PHONY: all clean install uninstall
