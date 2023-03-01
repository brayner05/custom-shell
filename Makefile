CFLAGS := -Wall -g

myshell: *.c
	$(CC) $(CFLAGS) $^ -o $@