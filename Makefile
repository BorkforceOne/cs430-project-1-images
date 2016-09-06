CC = gcc -O3
FILES = ppmrw.c
OUT_EXE = ppmrw

build: $(FILES)
	$(CC) -o $(OUT_EXE) $(FILES)

clean:
	rm -f $(OUT_EXE)

rebuild: clean build
