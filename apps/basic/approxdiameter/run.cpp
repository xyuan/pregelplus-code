#include "pregel_app_approxdiameter.h"


int main(int argc, char* argv[])
{
    init_workers();
    pregel_approxdiameter("/pullgel/livej", "/exp/approx", true);
    worker_finalize();
    return 0;
}
