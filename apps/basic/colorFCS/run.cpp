#include "pregel_app_colorFCS.h"

int main(int argc, char* argv[])
{
    init_workers();
    pregel_colorFCS("/pullgel/iusa", "/exp/cc");
    worker_finalize();
    return 0;
}
