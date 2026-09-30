# compiles the project - Jana create

CXX = g++ #C++ compiler

#compiler settings needed
#compile as c++17, show common warnings, additional warnign
#dont optimize, look inside include folder for prefix_sum.h
CXXFLAGS = -std=c++17 -Wall -Wextra -O0 -Iinclude 

#build required executable
all:
	$(CXX) $(CXXFLAGS) src/main.cpp src/worker.cpp -o my-sum

#terminal executable - g++ -std=c++17 -Wall -Wextra -O0 -Iinclude src/main.cpp src/worker.cpp -o my-sum

#remove the compiled executable
clean:
	rm -f my-sum