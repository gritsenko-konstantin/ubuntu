# Sccsid %Z% %M% %I% %G%
# 
# Make the DB-Library sample programs.
#
# Change the following definitions as appropriate for your site:

#CPPINC=/usr/vacpp/include
MQINC=/opt/mqm/inc
CPPINC=/usr/lib/gcc/x86_64-linux-gnu/4.7/include
CC = g++
APPLIBDIR = $(APPL)/lib

INC=-I. -I$(HOME)/api/inc 

DBLIBS = $(APPLIBDIR)/libsmpcom.a  $(APPLIBDIR)/api.a  $(MQLIB)/libmqm.so  -L/usr/mqm/lib64  $(MQLIB)/libimqb23gl.so  $(MQLIB)/libimqb23gl_r.so  $(MQLIB)/libimqs23gl.so  $(MQLIB)/libimqs23gl_r.so

all: apirecv apisend

apirecv: apirecv.cpp
	$(CC)   $(INC) apirecv.cpp $(DBLIBS) -lm -lnsl -o apirecv
 
apisend: apisend.cpp
	$(CC)	$(INC) apisend.cpp $(DBLIBS) -lm -lnsl -o apisend
clean:
	-rm -f apirecv apisend
