all: heap_mq posix_mq

heap_mq: heap_mq.c
	gcc heap_mq.c -o heap_mq -lpthread -lrt
	
posix_mq: posix_mq.c
	gcc posix_mq.c -o posix_mq -lpthread -lrt

clean:
	rm -f heap_mq posix_mq
