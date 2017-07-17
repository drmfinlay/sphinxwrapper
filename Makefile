TARGET = sphinxwrapper.so

CFLAGS=-fPIC `pkg-config --cflags --libs pocketsphinx sphinxbase`
CC=gcc

$(TARGET): sphinxwrapper.c sphinxwrapper.h
	$(CC) $(CFLAGS) $^ -o $@ -shared -Wl,-soname,$(TARGET)

clean:
	rm -f *.o *.so $(TARGET)
