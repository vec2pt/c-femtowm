TARGET_EXEC = femtowm

CFLAGS := -Os -std=c99 -Wall -Wextra -Werror -Wpedantic
LIBS := -lxcb -lxcb-keysyms

all:
	$(CC) $(CFLAGS) femtowm.c $(LIBS) -o $(TARGET_EXEC)

clean:
	rm -f $(TARGET_EXEC)
