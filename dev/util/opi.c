#include "opi.h"
#include <mpi.h>

struct optiq_performance_index opi, max_opi;
struct optiq_debug_print odp;

struct optiq_performance_index * optiq_opi_get()
{
    return &opi;
}

void optiq_opi_collect()
{
    MPI_Reduce (&opi.transfer_time, &max_opi.transfer_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Reduce (&opi.notification_done_time, &max_opi.notification_done_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Reduce (&opi.sendimm_time, &max_opi.sendimm_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Reduce (&opi.local_mem_req_time, &max_opi.local_mem_req_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Reduce (&opi.total_mem_req_time, &max_opi.total_mem_req_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Reduce (&opi.recv_len, &max_opi.recv_len, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    MPI_Reduce (&opi.matching_procesing_header_mr_response_time, &max_opi.matching_procesing_header_mr_response_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Reduce (&opi.context_advance_time, &max_opi.context_advance_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Reduce (&opi.post_rput_time, &max_opi.post_rput_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Reduce (&opi.check_complete_rput_time, &max_opi.check_complete_rput_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Reduce (&opi.get_header_time, &max_opi.get_header_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
}

void optiq_opi_print()
{
    double max_time =  max_opi.transfer_time / opi.iters;
    double bw = (double) max_opi.recv_len / max_time / 1024 / 1024 * 1e6;
    printf("\n");
    if (max_opi.recv_len < 1024) {
	printf("OPTIQ_Alltoallv: total_data = %ld (B) t = %8.4f, bw = %8.4f\n", max_opi.recv_len, max_time, bw);
    }
    else if (max_opi.recv_len < 1024 * 1024) {
	printf("OPTIQ_Alltoallv: total_data = %ld (KB) t = %8.4f, bw = %8.4f\n", max_opi.recv_len/1024, max_time, bw);
    }
    else {
	printf("OPTIQ_Alltoallv: total_data = %ld (MB) t = %8.4f, bw = %8.4f\n", max_opi.recv_len/1024/1024, max_time, bw);
    }

    if (odp.print_elapsed_time)
    {
	printf("context_advance_time time is %8.4f\n", max_opi.context_advance_time);
	printf("matching_procesing_header_mr_response_time time is %8.4f\n", max_opi.matching_procesing_header_mr_response_time);
	printf("get_header_time time is %8.4f\n", max_opi.get_header_time);
	printf("post_rput_time time is %8.4f\n", max_opi.post_rput_time);
	printf("check_complete_rput_time time is %8.4f\n", max_opi.check_complete_rput_time);
	printf("notification done time is %8.4f\n", max_opi.notification_done_time);
	printf("send_immediate time is %8.4f\n", max_opi.sendimm_time);
	printf("local mem req time is %8.4f\n", max_opi.local_mem_req_time);
	printf("total mem req time is %8.4f\n", max_opi.total_mem_req_time);
    }
    printf("\n");
}

void optiq_opi_clear()
{
    opi.recv_len = 0;
    opi.sendimm_time  = 0;
    opi.notification_done_time = 0;
    opi.transfer_time = 0;
    opi.build_path_time = 0;
    opi.context_advance_time = 0;
    opi.matching_procesing_header_mr_response_time = 0;
    opi.get_header_time = 0;
    opi.post_rput_time = 0;
    opi.check_complete_rput_time = 0;
    opi.paths.clear();
    opi.timestamps.clear();
    opi.total_mem_req_time = 0;
    opi.local_mem_req_time = 0;

    odp.print_path_id = false;
    odp.print_path_rank = false;
    odp.print_rput_msg = false;
    odp.print_debug_msg = false;
    odp.print_timestamp = false;
    odp.print_reduced_paths = false;
    odp.print_local_jobs = false;
    odp.print_sourcedests_id = false;
    odp.print_sourcedests_rank = false;
    odp.print_mem_reg_msg = false;
    odp.print_mem_exchange_status = false;
    odp.print_pami_transport_status = false;

    odp.test_mpi_perf = true;

    odp.print_elapsed_time = false;
}

void optiq_opi_timestamp_print(int rank)
{
    if (opi.timestamps.size() == 0) {
	return;
    }

    timeval t0 = opi.timestamps[0].tv;
    timeval t1;
    double t = 0;
    int eventid;
    int eventtype;

    for (int i = 1; i < opi.timestamps.size(); i++)
    {
	t1 = opi.timestamps[i].tv;
	eventid = opi.timestamps[i].eventid;
	eventtype = opi.timestamps[i].eventtype;

	t = (t1.tv_sec - t0.tv_sec) * 1e6 + (t1.tv_usec - t0.tv_usec);
	printf("rank = %d %d %d %8.4f\n", rank, eventtype, eventid, t);
    }
}
