
test_console: src/test.c src/controls.c src/timing.c
	gcc $^ -Iinclude/ -lncurses -o test 
