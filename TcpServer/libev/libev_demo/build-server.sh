LD_RUN_PATH=/usr/local/lib/
export LD_RUN_PATH
g++ -LLIBDIR -o server echo-server.cpp -lev -g
