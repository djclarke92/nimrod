CC=c++
CFLAGS=-c -Wall -g -I/usr/include/modbus -I/usr/include/mariadb -I/usr/include/openssl \
	-fmessage-length=0 -grecord-gcc-switches -D_FORTIFY_SOURCE=2 -fstack-protector -funwind-tables -fasynchronous-unwind-tables \
	-DOPENSSL_LOAD_CONF -DPIC -fPIC -fno-strict-aliasing -g -O -DNDEBUG -MMD -MP
CPPFLAGS += -MD -MP
LDFLAGS=-L/usr/lib64 -lmodbus -lmariadbclient -lpthread -lz -lm -lssl -lcrypto -ldl -lrt -lwebsockets
EXE=nimrod
SAEXE=set-address
CLEXE=nimrod-msg
SRCS=websockets.cpp cameras.cpp plcstates.cpp mb_main.cpp mb_utils.cpp mb_devices.cpp mb_mysql.cpp mb_thread.cpp
SASRCS=set-address.cpp
CLSRCS=nimrod-msg.cpp
OBJDIR=obj
OBJS=$(SRCS:%.cpp=$(OBJDIR)/%.o)
SAOBJS=$(SASRCS:%.cpp=$(OBJDIR)/%.o)
CLOBJS=$(CLSRCS:%.cpp=$(OBJDIR)/%.o)
DEP=$(SRCS:%.cpp=$(OBJDIR)/%.d)
SADEP=$(SASRCS:%.cpp=$(OBJDIR)/%.d)
CLDEP=$(CLSRCS:%.cpp=$(OBJDIR)/%.d)

.DEFAULT_GOAL := all

all: $(OBJS) $(SAOBJS) $(CLOBJS) $(EXE) $(SAEXE) $(CLEXE)

include $(DEP)
-include $(SADEP)
-include $(CLDEP)

$(EXE): $(OBJS)
	bash -c "../scripts/build-num.sh"
	$(CC) $(CFLAGS) -c build-number.c -o $(OBJDIR)/build-number.o
	$(CC) $(LDFLAGS) $(OBJS) $(OBJDIR)/build-number.o -o $(EXE) 

$(SAEXE): $(SAOBJS)
	$(CC) $(LDFLAGS) $(SAOBJS) -o $(SAEXE) 

$(CLEXE): $(CLOBJS)
	$(CC) $(LDFLAGS) $(CLOBJS) -o $(CLEXE) 

$(OBJS): $(OBJDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@
    
$(SAOBJS): $(OBJDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@
    
$(CLOBJS): $(OBJDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o
	rm -rf $(OBJDIR)/*.o
	rm -rf $(DEP)
	rm -rf $(SADEP)
	rm -rf $(CLDEP)
	rm -rf $(EXE)
	rm -rf $(SAEXE)
	rm -rf $(CLEXE)

	
