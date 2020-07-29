CC=g++
CFLAGS=-std=c++17

INCLUDECADMIUM=-I ../../cadmium/include
INCLUDEDESTIMES=-I ../../DESTimes/include

#CREATE BIN AND BUILD FOLDERS TO SAVE THE COMPILED FILES DURING RUNTIME
bin_folder := $(shell mkdir -p bin)
build_folder := $(shell mkdir -p build)
results_folder := $(shell mkdir -p simulation_results)

#TARGET TO COMPILE ALL THE TESTS TOGETHER (NOT SIMULATOR)

default: simulator

main_top.o: top_model/main.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) top_model/main.cpp -o build/main_top.o

cleaner_test.o: test/cleaner_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/cleaner_test.cpp -o build/cleaner_test.o

shipping_test.o: test/shipping_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/shipping_test.cpp -o build/shipping_test.o

hospital_test.o: test/hospital_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/hospital_test.cpp -o build/hospital_test.o



tests: cleaner_test.o shipping_test.o hospital_test.o
	$(CC) -g -o bin/cleaner_test build/cleaner_test.o
	$(CC) -g -o bin/shipping_test build/shipping_test.o
	$(CC) -g -o bin/hospital_test build/hospital_test.o

#TARGET TO COMPILE ONLY ABP SIMULATOR
simulator: main_top.o
	$(CC) -g -o bin/Laundry build/main_top.o

#TARGET TO COMPILE EVERYTHING (ABP SIMULATOR + TESTS TOGETHER)
all: simulator tests

#CLEAN COMMANDS
clean:
	rm -f bin/* build/*
