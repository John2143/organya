CC := g++ -std=c++11 -g
CFLAGS := -Wall -Wextra -Iinclude -Wno-unused-parameter -Wno-write-strings -Wno-char-subscripts -Wno-unused-but-set-variable -Wno-unused-variable

SRC=source

SRCS=$(wildcard $(SRC)/*.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRCS))
DEPS=$(patsubst %.cpp,%.d,$(SRCS))

RCSS=$(wildcard $(SRC)/*.rc)
RCOS=$(patsubst %.rc,%.o,$(RCSS))
RCTS=$(patsubst %.rc,%.rct,$(RCSS))

EXECUTABLE := organya

LIBRARIES:= -L. -lWinMM -ldsound -mwindows
ifeq ($(shell uname),Linux)
LIBRARIES += -lm
endif

.PHONY: clean all

# Default behaviour is to make the executable
all: $(EXECUTABLE)

# This is the final compile step:
#   take all the object files in $(SRC)/*.o and link them with the libraries
$(EXECUTABLE): $(OBJS) $(RCTS)
	$(CC) $(OBJS) $(RCOS) $(LIBRARIES) -o $(EXECUTABLE)

# To generate the .d files, I use the -M* options.
DEPFLAGS = -MT $@ -MMD -MP -MF $(SRC)/$*.d
COMPILE.cpp = $(CC) $(DEPFLAGS) $(CFLAGS)

# To create a .o file, take the source code and create 2 files: the .o with the
#  compiled code, and the .d with the dependency info
$(SRC)/%.o: $(SRC)/%.cpp
	$(COMPILE.cpp) -c $< -o $@

$(SRC)/%.rct: $(SRC)/%.rc
	windres $< $(SRC)/$*.o
	cp $(SRC)/$*.o $(SRC)/$*.rct

# if the dependency doesnt exist, we dont have to worry because that means the
#  corresponding object also doesnt exist. if the .d does exist then it will
#  contain info about dependant header files. It will look something like this:
#
#     $(SRC)/main.o: $(SRC)/main.cpp include/global.h include/graphics.h
#     include/global.h:
#     include/graphics.h:
#     include/renderScene.h:
#     include/input.h:
#
#   meaning that if include/global.h, or include/graphics.h were updated (newer
#   modified time than the .o file) then the above rule will need to be re run
#   again, causing a recompile, then the recompile causes a relink and the new
#   program is generated.
#
# Because the deps are optional, it may seem correct to use
#
#     -include $(DEPS)
#
#   but this is wrong becuase it will search in system directores for it as
#   well. You can see this with `strace make -n 2>&1 | grep /include/src`.
#   Wildcard will only include them if they exist.
#
include $(wildcard $(DEPS))


clean:
	rm -f $(SRC)/*.o
	rm -f $(SRC)/*.d
	rm -f $(SRC)/*.rct
	rm -f $(EXECUTABLE).exe
	rm -f $(EXECUTABLE)
