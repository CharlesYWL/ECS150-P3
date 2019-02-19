targets	:= libuthread.a
objs	:= sem.o tps.o #thread.o queue.o 

CC	:= gcc
CFLAGS	:= -Wall -Werror
CFLAGS	:= -g

ifneq ($(V),1)
Q = @
endif

all: $(targets)
AR=ar
ARFLAGS= rcs
deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)
DEPFLAGS = -MMD -MF $(@:.o=.d)
CUR_PWD := $(shell pwd)

$(targets): $(objs)
	@echo "AR	$@"
	$(Q)$(AR) $(ARFLAGS) $(targets) $^

%.o: %.c
	@echo "CC	$@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $< $(DEPFLAGS)


clean:
	@echo "CLEAN	$(CUR_PWD)"
	$(Q)rm -rf $(objs) $(deps) $(targets)
