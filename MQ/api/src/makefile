#CPPINC=/usr/vacpp/include
MQINC=/opt/mqm/inc
CPPINC=/usr/lib/gcc/x86_64-linux-gnu/4.7/include


INC=-I. -I$(CPPINC) \
	-I$(MQINC) \
	-I/usr/include \
	-I$(HOME)/api/inc \
	-I$(HOME)/api


MQLIB= -L/usr/lib -lmqm
SYSLIB= $(MQLIB) -lpthread 

#UTIL_LIB= $(HOME)/smpferun/lib
UTIL_LIB = /home/smprun/user/ubuntu/MQ/api/lib
#USERLIB= $(HOME)/smpferun/lib/api.a

#CC=/usr/vacpp/bin/xlC $(INC)   -qrtti=all -c -o
#CCR=/usr/vacpp/bin/xlC -D_DEBUG -D_AIX   -qrtti=all  -o 

CC = g++
CCRR = g++ -D_DEBUG -o

.SUFFIXES: .cpp .c .h .o

.cpp.o:
	$(CC) -c  $*.cpp $(INC)

.c.o:
	$(CC) -c  $*.cpp $(INC)

.C.o:
	$(CC) -c  $*.cpp $(INC)

all: api.a

api.a:	cstring.o cfile.o api.o mq.o mqagent.o MySemaphore.o PoolAccess.o sysrefdata.o
	ar rv $(UTIL_LIB)/api.a cstring.o cfile.o api.o mq.o mqagent.o  MySemaphore.o PoolAccess.o sysrefdata.o

apisend: apisend.o
	${CCRR} apisend apisend.o $(UTIL_LIB)/api.a  $(SYSLIB)

apirecv: apirecv.o
	${CCRR} apirecv apirecv.o $(UTIL_LIB)/api.a  $(SYSLIB)

clean:
	-rm *.o
