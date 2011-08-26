C = gcc
CFLAGS = -Wall -Werror -m32 -std=c99 -I . `pkg-config --cflags libglfw` -I/usr/include/lua5.1  -Isrc
LFLAGS = -m32 -Wl,--no-warn-search-mismatch
LIBS = -lGLU -L/usr/lib -L/usr/local/lib -llua `pkg-config --libs libglfw`
EXECUTABLE = vitae
include Makelist
#DEPS = $(SRCS:src/%.c=bin/%.d)
OBJS = $(SRCS:src/%.c=bin/%.o)
OBJS_DBG = $(SRCS:src/%.c=bin/debug/%.o)

all : $(EXECUTABLE)

# pull in dependency info for *existing* .o files
#-include $(OBJS:.o=.d)
-include $(SRCS:src/%.c=bin/%.d)

clean :
	@echo "--- Removing Object Files ---"
	@-rm -vf bin/*.o;
	@-rm -vf bin/*/*.o;
	@echo "--- Removing Executable ---"
	@-rm -vf $(EXECUTABLE);

cleandebug : 
	@echo "--- Removing Object Files ---"
	@-rm -vf bin/debug/*.o;
	@-rm -vf bin/debug/*/*.o;
	@echo "--- Removing Debug Executable ---"
	@-rm -vf $(EXECUTABLE)_debug;

$(EXECUTABLE) : $(SRCS) $(OBJS)
	@echo "- Linking $@"
	@$(C) $(LFLAGS) -O2 -o $(EXECUTABLE) $(OBJS) $(LIBS)


debug : $(EXECUTABLE)_debug

$(EXECUTABLE)_debug : $(SRCS) $(OBJS_DBG)
	@echo "- Linking $@"
	@$(C) -g $(LFLAGS) -o $(EXECUTABLE)_debug $(OBJS_DBG) $(LIBS)

bin/debug/%.o : src/%.c
	@mkdir -p bin/debug
	@mkdir -p bin/debug/camera
	@mkdir -p bin/debug/external
	@mkdir -p bin/debug/mem
	@mkdir -p bin/debug/render
	@mkdir -p bin/debug/system
	@echo "- Compiling $@"
	@$(C) -g $(CFLAGS) -D DEBUG -c -o $@ $<

bin/%.o : src/%.c
	@mkdir -p bin
	@mkdir -p bin/camera
	@mkdir -p bin/external
	@mkdir -p bin/mem
	@mkdir -p bin/render
	@mkdir -p bin/system
	@echo "- Compiling $@"
	@$(C) $(CFLAGS) -O2 -MD -c -o $@ $<
#	$(C) -MM $(CFLAGS) -O2 -MD -c $< > $*.d.tmp
#	sed -e 's/.*:/bin\/$*.o:/' < $*.d.tmp > bin/$*.d
#	rm $*.d.tmp

#bin/%.d : 
#	@
