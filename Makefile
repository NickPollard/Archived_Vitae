C = gcc
CFLAGS = -Wall -Wextra -Werror -m32 -std=c99 -I . `pkg-config --cflags libglfw` -I/usr/include/lua5.1  -Isrc -pg
LFLAGS = -m32 -Wl,--no-warn-search-mismatch -pg
LIBS = -lGLU -L/usr/lib -L/usr/local/lib -llua `pkg-config --libs libglfw`
EXECUTABLE = vitae
include Makelist
OBJS = $(SRCS:src/%.c=bin/release/%.o)
OBJS_DBG = $(SRCS:src/%.c=bin/debug/%.o)

all : $(EXECUTABLE)

# pull in dependency info for *existing* .o files
#-include $(OBJS:.o=.d)
-include $(SRCS:src/%.c=bin/release/%.d)
-include $(SRCS:src/%.c=bin/debug/%.d)

.PHONY : clean cleandebug android

clean :
	@echo "--- Removing Object Files ---"
	@find bin/release -name '*.o' -exec rm -vf {} \;
	@echo "--- Removing Executable ---"
	@-rm -vf $(EXECUTABLE);

cleandebug : 
	@echo "--- Removing Object Files ---"
	@-rm -vf bin/debug/*.o;
	@-rm -vf bin/debug/*/*.o;
	@echo "--- Removing Debug Executable ---"
	@-rm -vf $(EXECUTABLE)_debug;

android : 
	@echo "--- Building Native Code for Android NDK ---"
	@ndk-build -C android NDK_DEBUG=1 APP_OPTIM=debug
	@echo "--- Compiling Android Java and packaging APK ---"
	@ant debug -q -f android/build.xml
	@echo "--- Installing APK to device ---"
	@android/install.sh

$(EXECUTABLE) : $(SRCS) $(OBJS)
	@echo "- Linking $@"
	@$(C) $(LFLAGS) -O2 -o $(EXECUTABLE) $(OBJS) $(LIBS)


debug : $(EXECUTABLE)_debug

$(EXECUTABLE)_debug : $(SRCS) $(OBJS_DBG)
	@echo "- Linking $@"
	@$(C) -g $(LFLAGS) -o $(EXECUTABLE)_debug $(OBJS_DBG) $(LIBS)

bin/debug/%.o : src/%.c
#	Calculate the directory required and create it
	@mkdir -pv `echo "$@" | sed -e 's/\/[^/]*\.o//'`
	@echo "- Compiling $@"
	@$(C) -g $(CFLAGS) -MD -D DEBUG -c -o $@ $<

bin/release/%.o : src/%.c
#	Calculate the directory required and create it
	@mkdir -pv `echo "$@" | sed -e 's/\/[^/]*\.o//'`
	@echo "- Compiling $@"
	@$(C) $(CFLAGS) -O2 -MD -c -o $@ $<
