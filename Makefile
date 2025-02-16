CC = g++

CFLAGS = -Wall -Wextra -Werror -g  -std=c++98 -I./includes -I./includes/utils

TARGET = webserver

SRCS = 	main.cpp \
		src/Server.cpp \
		src/Client.cpp \
		src/Response.cpp \
		src/Request.cpp \
		src/Parser.cpp \
		src/Booter.cpp \
		src/ServerManager.cpp \
		src/Utils.cpp \
		src/utils/utilsServerManager.cpp \
		src/utils/utilsParser.cpp

all: $(TARGET)

configParser:
	c++ test/ConfigParser.cpp test/main.cpp ./src/*.cpp ./src/utils/*.cpp -I. -I./includes -Wall -Wextra -Wextra --std=c++98 -o  testConfigParser 

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

re: clean
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)


.PHONY: all clean