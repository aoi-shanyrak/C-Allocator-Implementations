CC = gcc
CFLAGS = -Wall -Wextra -g -O0 -std=c11 -Isrc
SANITIZERS = -fsanitize=address,undefined,leak

KR_TEST = test_kr
SPPOOL_TEST = test_sppool

all: $(KR_TEST) $(SPPOOL_TEST)

$(KR_TEST): src/tester.c src/kr_alloc.c src/kr_alloc.h
	$(CC) $(CFLAGS) -DTEST_KR_ALLOC -o $@ $^

$(SPPOOL_TEST): src/tester.c src/sppool_alloc.c src/sppool_alloc.h
	$(CC) $(CFLAGS) -o $@ $^

asan_kr: src/tester.c src/kr_alloc.c src/kr_alloc.h
	$(CC) $(CFLAGS) $(SANITIZERS) -DTEST_KR_ALLOC -o test_kr_asan $^
	@./test_kr_asan

asan_sppool: src/tester.c src/sppool_alloc.c src/sppool_alloc.h
	$(CC) $(CFLAGS) $(SANITIZERS) -o test_sppool_asan $^
	@./test_sppool_asan

asan: asan_kr asan_sppool

test: all
	@./$(KR_TEST)
	@./$(SPPOOL_TEST)

valgrind_kr: $(KR_TEST)
	valgrind --leak-check=full --show-leak-kinds=all ./$(KR_TEST)

valgrind_sppool: $(SPPOOL_TEST)
	valgrind --leak-check=full --show-leak-kinds=all ./$(SPPOOL_TEST)

valgrind: valgrind_kr valgrind_sppool

clean:
	rm -f $(KR_TEST) $(SPPOOL_TEST) test_kr_asan test_sppool_asan
	rm -f *.log *.txt

.PHONY: all test asan valgrind clean help
