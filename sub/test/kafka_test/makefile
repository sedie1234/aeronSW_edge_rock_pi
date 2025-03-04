# Makefile for Kafka Producer and Consumer

CXX = g++
CXXFLAGS = -std=c++11 -Wall
LDFLAGS = -lrdkafka++ -lrdkafka

SRC1 = producer.cpp
SRC2 = consumer.cpp

TARGET1 = producer
TARGET2 = consumer

all: $(TARGET1) $(TARGET2)

$(TARGET1): $(SRC1)
	$(CXX) $(CXXFLAGS) -o $(TARGET1) $(SRC1) $(LDFLAGS)

$(TARGET2): $(SRC2)
	$(CXX) $(CXXFLAGS) -o $(TARGET2) $(SRC2) $(LDFLAGS)

clean:
	rm -f $(TARGET1) 
