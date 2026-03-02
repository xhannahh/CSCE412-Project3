CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Wextra -pedantic-errors

TARGET = loadbalancer
SRCS = main.cpp LoadBalancer.cpp WebServer.cpp Firewall.cpp BonusLoadBalancer.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

docs:
	doxygen Doxyfile

.PHONY: all clean run docs
