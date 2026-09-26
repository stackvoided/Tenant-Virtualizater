TARGET_NAME = TenantVirtualizer
TARGET_SO   = $(TARGET_NAME).so
TARGET_DLL  = $(TARGET_NAME).dll

CXX         ?= g++
WIN_CXX     ?= i686-w64-mingw32-g++

SRCS        = JitHooks.cpp \
              NetworkRouter.cpp \
              SkeletalRaycast.cpp \
              TenantManager.cpp \
              main.cpp

OBJS_LINUX  = $(patsubst %.cpp, build/linux/%.o, $(SRCS))
OBJS_WIN    = $(patsubst %.cpp, build/win/%.o, $(SRCS))

COMMON_FLAGS = -m32 -std=c++20 -O3 -mavx2 -fPIC -Wall -Wextra -I.
LDFLAGS      = -m32 -shared

LINUX_CFLAGS  = $(COMMON_FLAGS) -fvisibility=hidden
LINUX_LDFLAGS = $(LDFLAGS) -pthread -ldl

WIN_CFLAGS    = $(COMMON_FLAGS)
WIN_LDFLAGS   = $(LDFLAGS) -static-libgcc -static-libstdc++ -lwinmm

.PHONY: all linux windows clean help

all: linux

linux: $(TARGET_SO)

$(TARGET_SO): $(OBJS_LINUX)
	$(CXX) $(OBJS_LINUX) $(LINUX_LDFLAGS) -o $@

build/linux/%.o: %.cpp | create_dirs
	$(CXX) $(LINUX_CFLAGS) -c $< -o $@

windows: $(TARGET_DLL)

$(TARGET_DLL): $(OBJS_WIN)
	$(WIN_CXX) $(OBJS_WIN) $(WIN_LDFLAGS) -o $@

build/win/%.o: %.cpp | create_dirs
	$(WIN_CXX) $(WIN_CFLAGS) -c $< -o $@

create_dirs:
	@mkdir -p build/linux build/win

clean:
	@rm -rf build $(TARGET_SO) $(TARGET_DLL)

help:
	@echo "linux / windows / clean"
