myls:myls.c libmylist.a
	gcc myls.c ./libmylist.a -g -o myls -Wall
.PHONY:clean
clean:
	rm myls 
