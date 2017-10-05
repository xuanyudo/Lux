ROOT=../..
DEVMAN_FOLDER=$(ROOT)/src/hub/devman

devman_test: $(DEVMAN_FOLDER)/devman.h $(DEVMAN_FOLDER)/devman.cpp devman_test.cpp
	g++ -std=c++11 -o devman_test.exe $(DEVMAN_FOLDER)/devman.h $(DEVMAN_FOLDER)/devman.cpp devman_test.cpp
	
clean:
	rm -rf devman_test.exe