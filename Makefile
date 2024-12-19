CC = c++

CFLAGS = -Wall -Wextra -Werror -g -std=c++98 -I./includes -I./includes/msg

SRCS = 	src/main.cpp \
		src/Server.cpp \
		src/Client.cpp \
		src/msg/Response.cpp \
		src/msg/_200.cpp \
		src/msg/_400.cpp 




TARGET = webserver

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

.PHONY: all clean