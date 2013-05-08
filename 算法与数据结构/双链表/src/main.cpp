#include "double_list.h"

using namespace mydatastructure;

int main(int argc, char** argv)
{
    DoubleList double_list;
    double_list.PushFront(10);
    double_list.PushFront(1);
    double_list.PushFront(5);
    double_list.PushFront(9);
    double_list.PushFront(200);

    double_list.Print();

    return 0;
}