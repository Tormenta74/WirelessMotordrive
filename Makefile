
test_console: console/test.c console/controls.c console/timing.c
	gcc $^ -Iinclude/ -lncurses -o test 
