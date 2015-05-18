INCLOOST = /home/osetr/boost_1_57_0/install-dir/include
LIBOOST = /home/osetr/boost_1_57_0/install-dir/lib

all:
	g++  hash.h keccak.h keccak.cpp main.cpp -lboost_filesystem -lboost_filesystem -lboost_system -I$(INCLOOST) -L$(LIBOOST) -std=c++11 -o la
