CC = c++

CFLAGS = -Wall -Wextra -Werror -g -std=c++98 -I./includes 

SRCS = 	src/main.cpp \
		src/Server.cpp \
		src/Client.cpp


TARGET = webserver

# Default target
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Clean up build files
clean:
	rm -f $(TARGET)

# Phony targets
.PHONY: all clean