CC=c++
CFLAGS=-c -Wall -g -I/usr/include/modbus -I/usr/include/mariadb -I/usr/include/openssl \
	-fmessage-length=0 -grecord-gcc-switches -D_FORTIFY_SOURCE=2 -fstack-protector -funwind-tables -fasynchronous-unwind-tables \
	-DOPENSSL_LOAD_CONF -DPIC -fPIC -fno-strict-aliasing -g -O -DNDEBUG -MMD -MP
LDFLAGS=-L/usr/lib64 -lmodbus -lmariadbclient -lpthread -lz -lm -lssl -lcrypto -ldl -lrt
EXE=nimrod
SAEXE=set-address
CLEXE=nimrod-msg
SRCS=mb_main.cpp mb_utils.cpp mb_devices.cpp mb_mysql.cpp mb_thread.cpp
SASRCS=set-address.cpp
CLSRCS=nimrod-msg.cpp
OBJS=$(SRCS:.cpp=.o) build-number.o
SAOBJS=$(SASRCS:.cpp=.o)
CLOBJS=$(CLSRCS:.cpp=.o)
DEP=$(SRCS:.cpp=.d)
SADEP=$(SASRCS:.cpp=.d)
CLDEP=$(CLSRCS:.cpp=.d)
-include $(DEP)
-include $(SADEP)
-include $(CLDEP)

BUILD_NUMBER_FILE=build-number.c
OBJECTS=build-number.o

all: $(SRCS) $(SASRCS) $(CLSRCS) $(EXE) $(SAEXE) $(CLEXE)

$(EXE): $(OBJS) $(BUILD_NUMBER_FILE)
	$(CC) $(LDFLAGS) $(OBJS) -o $(EXE) 

$(SAEXE): $(SAOBJS)
	$(CC) $(LDFLAGS) $(SAOBJS) -o $(SAEXE) 

$(CLEXE): $(CLOBJS)
	$(CC) $(LDFLAGS) $(CLOBJS) -o $(CLEXE) 

.cpp.o: 
	$(CC) $(CFLAGS) $< -o $@
    
$(BUILD_NUMBER_FILE).o: $(BUILD_NUMBER_FILE).c
	bash -c "../scripts/build-num.sh"
	$(CC) $(CFLAGS) -c $(BUILD_NUMBER_FILE).c
	
clean:
	rm -rf *.o
	rm -rf $(DEP)
	rm -rf $(SADEP)
	rm -rf $(CLDEP)
	rm -rf $(EXE)
	rm -rf $(SAEXE)
	rm -rf $(CLEXE)

	
