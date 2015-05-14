#include "opi.h"
#include <mpi.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <algorithm>
#include <vector>

struct optiq_performance_index opi, max_opi;
struct optiq_debug_print odp;

void optiq_opi_init()
{
    int size, rank;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    if (rank == 0)
    {
        max_opi.all_numcopies = (int *) calloc (1, sizeof(int) * size);
        max_opi.all_numrputs = (int *) calloc (1, sizeof(int) * size);

        max_opi.all_link_loads = (int *) calloc (1, sizeof(int) * size * 9);
    }

    opi.load_stat = NULL;

    optiq_opi_clear();
}

struct optiq_performance_index * optiq_opi_get()
{
    return &opi;
}

void optiq_opi_collect_map(std::map<int, int> &input, std::map<int, int> &output, struct optiq_stat &stat)
{
    int size, rank;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    int *counts = (int *) calloc (1, sizeof(int) * size);
    int *displs = (int *) calloc (1, sizeof(int) * size);

    int inputsize = input.size() * 2;
    MPI_Gather (&inputsize, 1, MPI_INT, counts, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int totalcounts = 0;
    for (int i = 0; i < size; i++)
    {
	displs[i] = totalcounts;
        totalcounts += counts[i];
    }

    int *allinput = NULL;
    if (rank == 0)
    {
        allinput = (int *) calloc (1, sizeof(int) * totalcounts);
    }

    int *inputfreq;
 
    if (inputsize > 0) 
    {    
	inputfreq  = (int*) calloc (1, sizeof(int) * inputsize);
    }

    std::map<int, int>::iterator it;

    int i = 0;
    for (it = input.begin(); it != input.end(); it++)
    {
        inputfreq[i] = it->first;
	inputfreq[i+1] = it->second;
	i += 2;
    }

    MPI_Gatherv(inputfreq, inputsize, MPI_INT, allinput, counts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
	output.clear();

	for (int i = 0; i < totalcounts/2; i++)
	{
	    it = output.find(allinput[i*2]);

	    if (it == output.end())
	    {
		output.insert(std::pair<int, int> (allinput[i*2], allinput[i*2+1]));
	    }
	    else
	    {
		it->second += allinput[i*2+1];
	    }
	}
    }

    stat.max = 0;
    stat.min = INT_MAX;
    stat.avg = 0;
    stat.total = 0;
    stat.med = 0;
    int medindex = 0;

    for (it = output.begin(); it != output.end(); it++)
    {
	stat.total += it->first * it->second;

	if (stat.min > it->first && it->first > 0)
	{
	    stat.min = it->first;
	}

	if (stat.max < it->first)
	{
	    stat.max = it->first;
	}

	medindex += it->second;
	if (medindex > opi.paths.size()/2 && stat.med == 0)
	{
	    stat.med = it->first;
	}
    }

    stat.avg = stat.total/opi.numpaths.total;

    if (inputsize > 0)
    {
	free(inputfreq);
    }
    if (rank == 0)
    {
	free(allinput);
    }
    free(counts);
    free(displs);
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

    max_opi.iters = opi.iters;

    int size, rank;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    int link_loads[9] = {}, in = 0;
    std::map<int, int>::iterator it;

    for (it  = opi.link_loads.begin(); it != opi.link_loads.end(); it++)
    {
	link_loads[in] = it->second;
	in++;
    }

    if (rank == 0)
    {
	memset (max_opi.all_numcopies, 0, sizeof(int) * size);
	memset (max_opi.all_numrputs, 0, sizeof(int) * size);
	memset (max_opi.all_link_loads, 0, sizeof(int) * size * 9);
    }

    MPI_Gather (&opi.numcopies, 1, MPI_INT, max_opi.all_numcopies, 1, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Gather (&opi.numrputs, 1, MPI_INT, max_opi.all_numrputs, 1, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Gather (link_loads, 9, MPI_INT, max_opi.all_link_loads, 9, MPI_INT, 0, MPI_COMM_WORLD);

    optiq_opi_collect_map(opi.path_copy, opi.path_copy, opi.hopcopy);
    optiq_opi_collect_map(opi.path_hopbyte, opi.path_hopbyte, opi.hopbyte);

    
}

void optiq_opi_compute_stat()
{
    int size, rank;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    if (rank == 0)
    {
	int mincopies = 0, maxcopies = 0, medcopies = 0, total_numcopies = 0;
	int minrputs = 0, maxrputs = 0, medrputs = 0, total_rputs = 0;
	int minlinkloads = 0, maxlinkloads = 0, medlinkloads = 0;
	long total_linkloads = 0;
	double avgcopies = 0, avglinkloads = 0, avgrputs = 0;

	std::vector<int> copies (max_opi.all_numcopies, max_opi.all_numcopies + size);
	std::sort (copies.begin(), copies.end());

	std::vector<int> rputs (max_opi.all_numrputs, max_opi.all_numrputs + size);
	std::sort (rputs.begin(), rputs.end());

	std::vector<int> linkloads (max_opi.all_link_loads, max_opi.all_link_loads + size * 9);
	std::sort (linkloads.begin(), linkloads.end());
	opi.linkloads = linkloads;

	int ncpy = 0, nrput = 0, nlinks = 0;
	maxcopies = copies[size - 1];
	maxrputs = rputs[size - 1];
	maxlinkloads = linkloads[size * 9 - 1];

	std::map<int, int>::iterator it;
	opi.copies_dist.clear();

	for (int i = size; i >= 0; i--)
	{
	    if (copies[i] != 0)
	    {
		total_numcopies += copies[i];
		mincopies = copies[i];
		ncpy++;

		it = opi.copies_dist.find(copies[i]);

		if (it == opi.copies_dist.end())
		{
		    opi.copies_dist.insert(std::pair<int, int>(copies[i], 1));
		}
		else
		{
		    it->second++;
		}
	    }

	    if (rputs[i] != 0)
	    {
		total_rputs += rputs[i];
		minrputs = rputs[i];
		nrput++;
	    }
	}

	for (int i = size * 9; i >= 0; i--)
	{
	    if (linkloads[i] != 0)
	    {
		total_linkloads += linkloads[i];
		minlinkloads = linkloads[i];
		nlinks++;
	    }
	}

	medcopies = copies[size - 1 - ncpy/2];
	avgcopies = (double) total_numcopies / ncpy;

	medrputs = rputs[size - 1 - nrput/2];
	avgrputs = (double) total_rputs / nrput;

	medlinkloads = linkloads[size * 9 - 1 - nlinks/2];
	avglinkloads = (double) total_linkloads / nlinks;

	opi.copies.max = maxcopies;
	opi.copies.min = mincopies;
	opi.copies.avg = avgcopies;
	opi.copies.med = medcopies;
	opi.copies.total = total_numcopies;
	
	opi.rputs.max = maxrputs;
	opi.rputs.min = minrputs;
	opi.rputs.avg = avgrputs;
	opi.rputs.med = medrputs;
	opi.rputs.total = total_rputs;

	opi.load_link.max = maxlinkloads;
	opi.load_link.min = minlinkloads;
	opi.load_link.avg = avglinkloads;
	opi.load_link.med = medlinkloads;
	opi.load_link.total = total_linkloads;
    }
}

void optiq_opi_print()
{
    printf("%s ", opi.prefix);

    printf(" %d ", opi.test_id);

    printf("%s ", opi.name);

    printf(" msg = %d chunk = %d ", opi.message_size, opi.chunk_size);

    double max_time =  max_opi.transfer_time / max_opi.iters;
    double bw = (double) max_opi.recv_len / max_time / 1024 / 1024 * 1e6;

    if (max_opi.recv_len < 1024) {
	printf(" %ld", max_opi.recv_len);
    }
    else if (max_opi.recv_len < 1024 * 1024) {
	printf(" %ld", max_opi.recv_len/1024);
    }
    else {
	printf(" %ld", max_opi.recv_len/1024/1024);
    }

    printf(" %8.4f %8.4f ", max_time, bw);

    printf(" %4.0f %d %d %4.2f %d ", opi.numpaths.total, opi.numpaths.max, opi.numpaths.min, opi.numpaths.avg, opi.numpaths.med);

    printf(" %4.0f %d %d %4.2f %d ", opi.hopbyte.total, opi.hopbyte.max, opi.hopbyte.min, opi.hopbyte.avg, opi.hopbyte.med);

    printf(" %4.0f %d %d %4.2f %d ", opi.hopcopy.total, opi.hopcopy.max, opi.hopcopy.min, opi.hopcopy.avg, opi.hopcopy.med);

    printf(" %4.0f %d %d %4.2f %d ", opi.hops.total, opi.hops.max, opi.hops.min, opi.hops.avg, opi.hops.med);

    printf(" %4.0f %d %d %4.2f %d ", opi.copies.total, opi.copies.max, opi.copies.min, opi.copies.avg, opi.copies.med);

    printf(" %d %d %4.2f %d ", opi.load_path.max, opi.load_path.min, opi.load_path.avg, opi.load_path.med);
    
    printf(" %d %d %4.2f %d ", opi.load_link.max, opi.load_link.min, opi.load_link.avg, opi.load_link.med);

    printf(" %4.0f %d %d %4.2f %d  ", opi.rputs.total, opi.rputs.max, opi.rputs.min, opi.rputs.avg, opi.rputs.med);

    printf("\n");

    std::map<int, int>::iterator it;

    /* Print copies distribution */
    for (it = opi.copies_dist.begin(); it != opi.copies_dist.end(); it++)
    {
	printf("Copy: %d nodes has %d copies\n", it->second, it->first);
    }

    /* Print hops distribution */
    for (it = opi.hops_dist.begin(); it != opi.hops_dist.end(); it++)
    {
        printf("Hop: %d paths has %d hops\n", it->second, it->first);
    }

    /* Print link load - num. of paths */
    for (int i = 0; opi.load_stat[i] != -1; i++)
    {
	printf("Path: %d links has %d paths\n", opi.load_stat[i], i);
    }

    /* Print link load - actual data size */
    std::map<int, int> linkloadfreq;
    linkloadfreq.clear();

    for (int i = opi.linkloads.size() - 1; i >= 0 && opi.linkloads[i] > 0; i--)
    {
	it = linkloadfreq.find(opi.linkloads[i]);

	if (it != linkloadfreq.end())
	{
	    it->second++;
	}
	else 
	{
	    linkloadfreq.insert(std::pair<int, int> (opi.linkloads[i], 1));
	}
    }

    for (it = linkloadfreq.begin(); it != linkloadfreq.end(); it++)
    {
	printf("Data: %d links has %d data size passed through\n", it->second, it->first);
    }

    optiq_opi_print_path_hopbyte_copy_stat();

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

void optiq_opi_print_path_hopbyte_copy_stat()
{
    std::map<int, int>::iterator it;

    for (it = opi.path_hopbyte.begin(); it != opi.path_hopbyte.end(); it++)
    {
	printf("B_hopbyte: %d paths has %d hopbytes\n", it->second, it->first);
    }

    for (it = opi.path_copy.begin(); it!= opi.path_copy.end(); it++)
    {
	printf("Y_path_copy: %d paths has %d copies\n", it->second, it->first);
    }
}

void optiq_opi_clear()
{
    for (int i = 0; i < opi.paths.size(); i++)
    {
	if (opi.paths[i] != NULL) {
	    free(opi.paths[i]);
	}
    }
    opi.paths.clear();

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
    odp.print_rput_rdone_notify_msg = false;
    odp.print_recv_rput_done_msg = false;
    odp.print_mem_adv_exchange_msg = false;
    odp.print_mem_avail = false;
    odp.test_mpi_perf = true;
    odp.print_notify_list = false;
    odp.print_elapsed_time = false;
    odp.print_job = false;
    odp.print_done_status = false;
    odp.print_transport_perf = false;
    odp.print_mpi_paths = false;

    odp.collect_transport_perf = false;
    odp.collect_timestamp = false;

    opi.numcopies = 0;
    opi.numrputs = 0;
    opi.link_loads.clear();
    opi.hops_dist.clear();
    opi.copies_dist.clear();
    opi.path_copy.clear();
    opi.path_hopbyte.clear();

    opi.load_link = optiq_stat();
    opi.load_path = optiq_stat();
    opi.hops = optiq_stat();
    opi.copies = optiq_stat();
    opi.numpaths = optiq_stat();
    opi.rputs = optiq_stat();
    opi.hopbyte = optiq_stat();
    opi.hopcopy = optiq_stat();

    if (opi.load_stat != NULL) 
    {
	free(opi.load_stat);
	opi.load_stat = NULL;
    }
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
