#include "pregel_app_sssp.h"
int main(int argc, char* argv[])
{
    init_workers();
    pregel_sssp(0, "/pullgel/usa", "/exp/usa/sssp_out", true);
    worker_finalize();
    return 0;
}
