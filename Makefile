.PHONY: all run stop clean

all: build run #complile and run all with correct commands

build: producer consumer

producer: producer.c common.h
	gcc producer.c -pthread -lrt -o producer

consumer: consumer.c common.h
	gcc consumer.c -pthread -lrt -o consumer

run: producer consumer 
	@echo "Starting producer and consumer..."
	./producer & ./consumer &
	@echo "Press Ctrl+C to observe blocking behavior, then run 'make stop' to terminate."

stop: #terminate processes
	- pkill -x producer || true
	- pkill -x consumer || true

clean: #clean up executables
	rm -f producer consumer