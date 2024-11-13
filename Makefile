PROJECTNAME=zipix

CC=gcc
LDFLAGS=-I./include/
CFLAGS=-O3

RELEASE_CFLAGS=-Wall -Wextra
TEST_CFLAGS=-Wall -Wextra -fsanitize=address
TEST_LDFLAGS=-lcriterion

MAIN_FILE=$(shell find src/ -type f -name "main.c")
RELEASE_SRC=$(shell find src/ -type f -name '*.c')
RELEASE_OBJ=$(subst src/,obj/,$(RELEASE_SRC:.c=.o))

TESTS_SRC=$(shell find tests/src/ -type f -name '*.c')
TESTS_OBJ=$(subst src/,obj/,$(TESTS_SRC:.c=.o))
UNITS_OBJ=$(subst obj/,tests/units/,$(RELEASE_OBJ))
MAIN_OBJ =$(subst src/,tests/units/,$(MAIN_FILE:.c=.o))
UNITS_OBJ:=$(filter-out $(MAIN_OBJ),$(UNITS_OBJ))
TESTS_BIN=$(subst src/,bin/,$(TESTS_SRC:.c=))

default: run-tests

obj/%.o: src/%.c
	mkdir -p obj &> /dev/null
	$(CC) $(RELEASE_CFLAGS) $(CFLAGS) -c $^ -o $@ $(LDFLAGS)

bin/$(PROJECTNAME): $(RELEASE_OBJ)
	mkdir -p bin &> /dev/null
	$(CC) $(RELEASE_CFLAGS) $(CFLAGS) $^ -o $@ $(LDFLAGS)

build: bin/$(PROJECTNAME)

tests/units/%.o: src/%.c
	mkdir -p tests/units &> /dev/null
	$(CC) $(TEST_CFLAGS) $(CFLAGS) -c $^ -o $@ $(LDFLAGS)

tests/obj/%.o: tests/src/%.c
	mkdir -p tests/obj &> /dev/null
	$(CC) $(TEST_CFLAGS) $(CFLAGS) -c $^ -o $@ $(LDFLAGS)

tests/bin/%: tests/obj/%.o $(UNITS_OBJ)
	mkdir -p tests/bin &> /dev/null
	$(CC) $(TEST_CFLAGS) $(CFLAGS) $^ -o $@ $(TEST_LDFLAGS) $(LDFLAGS)

# prevent deleting object in rules chain
$(TESTS_BIN): $(UNITS_OBJ) $(TESTS_OBJ)

run-tests: $(TESTS_BIN)
	./$^ || true

clean:
	rm -f $(RELEASE_OBJ) $(TESTS_OBJ)

clean-all: clean
	rm -f $(TESTS_BIN) $(UNITS_OBJ)
