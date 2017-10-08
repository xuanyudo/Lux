ROOT=../..
HUB_FOLDER=$(ROOT)/src/hub
JSON_FOLDER=$(ROOT)/src/utils/JSON
DEVMAN_FOLDER=$(HUB_FOLDER)/devman

FLAGS=-std=c++11 -pthread

HUB_FILES=$(HUB_FOLDER)/server.h $(HUB_FOLDER)/server.cpp
DEVMAN_FILES=$(DEVMAN_FOLDER)/devman.h $(DEVMAN_FOLDER)/devman.cpp
JSON_FILES=$(JSON_FOLDER)/json.h $(JSON_FOLDER)/json.cpp

ALL_FILES=$(JSON_FILES) $(DEVMAN_FILES) $(HUB_FILES) server_test.cpp

server_test: $(ALL_FILES)
	g++ $(FLAGS) -o server_test.exe $(ALL_FILES)
	
clean:
	rm -rf server_test.exe