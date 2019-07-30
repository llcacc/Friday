INCLUDE = -I. -I./speech
LIB= -ljsoncpp -lcurl -lcrypto -lpthread
friday: friday.cc
	g++ -o $@ $^ $(LIB) -std=c++11 $(INCLUDE) #-static
.PHONY:clean
clean:
	rm -f friday
