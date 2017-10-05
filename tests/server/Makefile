ROOT=../..
HUB_FOLDER=$(ROOT)/src/hub
UTILS_FOLDER=$(ROOT)/src/utils
DEVMAN_FOLDER=$(HUB_FOLDER)/devman

FLAGS=-std=c++11

HUB_FILES=$(HUB_FOLDER)/server.h $(HUB_FOLDER)/server.cpp
DEVMAN_FILES=$(DEVMAN_FOLDER)/devman.h $(DEVMAN_FOLDER)/devman.cpp
UTILS_FILES=$(UTILS_FOLDER)/Parser.h $(UTILS_FOLDER)/Parser.cpp $(UTILS_FOLDER)/encode.h $(UTILS_FOLDER)/encode.cpp

ALL_FILES=server_test.cpp $(UTILS_FILES) $(DEVMAN_FILES) $(HUB_FILES)

server_test: $(ALL_FILES)
	g++ $(FLAGS) -o server_test.exe $(ALL_FILES)
	
clean:
	rm -rf server_test.exe