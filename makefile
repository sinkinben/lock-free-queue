all:
	g++ -Wall -O3 -march=native -std=c++17 benchmark.cpp -lpthread -o spsc_queue.exe

test:
	g++ -Wall -O3 -march=native -std=c++17 test.cpp -lpthread -o test.exe
	./test.exe

clean:
	rm *.exe