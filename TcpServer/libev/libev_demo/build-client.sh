LD_RUN_PATH=/usr/local/lib/
export LD_RUN_PATH
g++ -LLIBDIR -o client echo-client.cpp -lev -g
