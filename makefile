myls:myls.c libmylist.a
	gcc myls.c ./libmylist.a -g -o myls -Wall
libmylist.a: mylist.o
	ar -rc libmylist.a mylist.o
mylist.o:mylist.c mylist.h
	gcc -g -c mylist.c -o mylist.o -Wall
.PHONY:clean
clean:
<<<<<<< HEAD
	rm myls  libmylist.a  mylist.o 
=======
<<<<<<< HEAD
	rm myls  libmylist.a  mylist.o 
=======
	rm myls  libmylist.a mylist.o 
>>>>>>> 51bf56b170e736bec2ed6fc6c9ddc211fbda6af9
>>>>>>> a394576365e1eb9d91f672332fa9557fbed9ad98
