all: Release
.DEFAULT_GOAL := Release

warns    := -Wall -pedantic -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Wmain -pedantic-errors
cflags   := $(warns) -I./include
cxxflags := $(warns) -I./include
cppflags :=
ldflags  := -lzip

sources  := $(wildcard src/*.cpp)
objdir   := obj

bmode := release
ifeq (Test, $(filter Test, $(MAKECMDGOALS)))
	bmode := test
else ifeq (test, $(filter test, $(MAKECMDGOALS)))
	bmode := test
else ifeq (Debug, $(filter Debug, $(MAKECMDGOALS)))
	bmode := debug
else ifeq (debug, $(filter debug, $(MAKECMDGOALS)))
	bmode := debug
endif

ifeq (test, $(bmode))
  cflags += -pg
  cxxflags += -pg -std=gnu++11 -Wno-variadic-macros
  sources += $(wildcard test/*.cpp)
  objdir := $(objdir)/Test
else ifeq (debug, $(bmode))
  cflags += -g
  cxxflags += -g
  sources += build/main.cpp
  objdir := $(objdir)/Debug
else
  cflags += -O3
  cxxflags += -O3
  ldflags += -s
  sources += build/main.cpp
  objdir := $(objdir)/Release
endif

objects := $(addprefix $(objdir)/,$(patsubst %.cpp, %.o, $(sources)))
objdirs = $(sort $(dir $(objects)))

OS := $(shell uname -s)
ifeq ($(OS),$(filter $(OS),GNU Linux FreeBSD Darwin))
	binName = EGMRead
	VG = valgrind --leak-check=full --track-origins=yes --error-exitcode=2
else
	binName = EGMRead.exe
	VG =
endif

bin/Debug:
	mkdir -p bin/Debug
bin/Release:
	mkdir -p bin/Release
bin/Test:
	mkdir -p bin/Test

$(objdirs):
	mkdir -p $@

$(objdir)/%.o $(objdir)/%.d: %.cpp | $(objdir)
	$(CXX) $(cxxflags) $(cppflags) -MMD -MP -c $< -o $(objdir)/$*.o

bin/Release/$(binName): $(objdirs) $(objects) bin/Release
	$(CXX) $(objects) $(relflags) $(ldflags) -o $@
bin/Debug/$(binName):   $(objdirs) $(objects) bin/Debug
	$(CXX) $(objects) $(dbgflags) $(ldflags) -o $@
bin/Test/$(binName):    $(objdirs) $(objects) bin/Test
	$(CXX) $(objects) $(relflags) $(ldflags) -o $@

Release: bin/Release/$(binName)
Debug:   bin/Debug/$(binName)
Test:    bin/Test/$(binName)
	cd test && $(VG) "../bin/Test/$(binName)"

release: Release
debug: Debug
test: Test

cleanDebug:
	rm -rf bin/Debug obj/Debug
cleanRelease:
	rm -rf bin/Release obj/Release
cleanTest:
	rm -rf bin/Test obj/Test
clean:
	rm -rf bin/ obj/
