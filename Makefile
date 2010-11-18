C = gcc
CFLAGS = -Wall -Werror -m32 -std=c99 -I . `pkg-config --cflags libglfw`
LFLAGS = -m32 
LIBS = -lGLU -llua `pkg-config --libs libglfw`
EXECUTABLE = vitae
include Makelist
DEPS = $(SRCS:src/%.c=bin/%.d)
OBJS = $(SRCS:src/%.c=bin/%.o)
OBJS_DBG = $(SRCS:src/%.c=bin/debug/%.o)

all : $(EXECUTABLE)

clean :
	@echo "--- Removing Object Files ---"
	@-rm -vf bin/*.o;
	@-rm -vf bin/*/*.o;
	@echo "--- Removing Executable ---"
	@-rm -vf $(EXECUTABLE);

cleandebug : 
	@echo "--- Removing Object Files ---"
	@-rm -vf bin/debug/*.o;
	@echo "--- Removing Debug Executable ---"
	@-rm -vf $(EXECUTABLE)_debug;

$(EXECUTABLE) : $(SRCS) $(OBJS) $(DEPS)
	@echo "- Linking $@"
	@$(C) $(LFLAGS) -O2 -o $(EXECUTABLE) $(OBJS) $(LIBS)

debug : $(EXECUTABLE)_debug

$(EXECUTABLE)_debug : $(SRCS) $(OBJS_DBG)
	@echo "- Linking $@"
	@$(C) -g $(LFLAGS) -o $(EXECUTABLE)_debug $(OBJS_DBG) $(LIBS)

bin/debug/%.o : src/%.c
	@echo "- Compiling $@"
	@$(C) -g $(CFLAGS) -c -o $@ $<

bin/%.o : src/%.c
	@echo "- Compiling $@"
	@$(C) $(CFLAGS) -O2 -MD -c -o $@ $<

bin/%.o : src/%.c
	@echo "- Compiling $@"
	@$(C) $(CFLAGS) -O2 -MD -c -o $@ $<

bin/%.d : 
	@
