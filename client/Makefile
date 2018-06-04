all:
	g++ crypto.cpp -g client.cpp \
        -lssl -lcrypto -lm -lpthread \
        -o client.o

.PHONY: clean
clean:
	@rm -f *.o
