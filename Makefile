build:
	g++ ./src/*.cpp -o ./bin/main `pkg-config --libs --cflags sdl3`
run:
	./bin/main
clean:
	rm -r ./bin/main


