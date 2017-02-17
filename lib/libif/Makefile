CC = arm-none-linux-gnueabi-gcc
INCLUDES = ../include
LIBS = 
CFLAGS = -shared -fPIC -g -o

target:
	cp ../libdevice_interface.a ./
		
	ar x libdevice_interface.a
	
	ar cr libhmui_event.a *.o
	ranlib libhmui_event.a

	cp libhmui_event.a ../../bin

	rm *.o *.a

clean:
	rm *.o
