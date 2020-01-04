#include <iostream>

#include "q_data_type.h"

using namespace std;
using namespace cactus;

int main(int argc, char const* argv[]) {
    Operation op(1, 5);
    cout << op << endl;
    Operation op1(1, 19);
    cout << op1 << endl;

    return 0;
}
