CC = g++

CFLAGS = -Wall -Wextra -Werror -g -std=c++98 -I./includes -I./includes/msg

SRCS = 	src/main.cpp \
		src/Server.cpp \
		src/Client.cpp \
		src/msg/Response.cpp \
		src/msg/Request.cpp \
		src/Parser.cpp \
		src/Booter.cpp \
		src/ServerManager.cpp \
		src/Utils.cpp

re: clean
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

TARGET = webserver

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

.PHONY: all clean