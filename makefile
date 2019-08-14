myls:myls.c libmylist.a
	gcc myls.c ./libmylist.a -g -o myls -Wall
libmylist.a: mylist.o
	ar -rc libmylist.a mylist.o
mylist.o:mylist.c mylist.h
	gcc -g -c mylist.c -o mylist.o -Wall
.PHONY:clean
clean:
	rm myls  libmylist.a  mylist.o 
