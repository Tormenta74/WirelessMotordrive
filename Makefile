
test_console: console/test.c console/controls.c
	gcc $^ -lncurses -o test 
