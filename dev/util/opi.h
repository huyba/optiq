#ifndef OPTIQ_PERFORMANCE_INDEX
#define OPTIQ_PERFORMANCE_INDEX

#include "path.h"

struct optiq_performance_index {
    double transfer_time;
    double build_path_time;
    double notification_done_time;
    double sendimm_time;
    double matching_procesing_header_mr_response_time;
    double get_header_time;
    double post_rput_time;
    double check_complete_rput_time;

    int long recv_len;
    int iters;

    int optiq_max_load;
    int optiq_min_load;
    int optiq_avg_load;
    int optiq_total_load;

    int optiq_max_hops;
    int optiq_min_hops;
    int optiq_avg_hops;
    int optiq_total_hops;

    std::vector<struct path *> paths;
};

extern "C" struct optiq_performance_index opi;

struct optiq_performance_index * optiq_opi_get();

void optiq_opi_collect(int world_rank);

void optiq_opi_clear();

#endif
