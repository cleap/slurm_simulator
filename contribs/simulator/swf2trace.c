//#ifdef SLURM_SIMULATOR
#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <limits.h>

#include "sim_trace.h"

#define CPUS_PER_NODE    16
static const char DEFAULT_OFILE[]    = "simple.trace"; //The output trace name should be passed as an argument on command line.

typedef struct swf_job_trace {
    int job_num;
    time_t sub_time;
    int wait_time;
    int run_time;
    int alloc_cpus;
    int avg_cpu_time;
    int mem;
    int req_cpus;
    int req_time;
    int req_mem;
    int status;
    int uid;
    int gid;
    int app_num;
    int queue_num;
    int part_num;
    int prev_job_num;
    int think_time_prev_job;
} swf_job_trace_t;


int main(int argc, char* argv[])
{
    int i;
    int idx = 0, errs = 0;
    int counter = 0, job_index = 0;
    int nrecs = 0;
    job_trace_t* job_trace,* job_trace_head,* job_arr,* job_ptr;
    swf_job_trace_t *swf_job_arr;
    char const* const fileName = argv[1]; /* should check that argc > 1 */
    FILE* file = fopen(fileName, "r"); /* should check the result */
    char line[256];
    char *p;
    char ch;
    while(!feof(file)) {
        ch = fgetc(file);
        if(ch == '\n')
        {
            nrecs++;
        }
    }
    rewind(file);
    job_arr = (job_trace_t*)malloc(sizeof(job_trace_t)*nrecs);
    if (!job_arr) {
                printf("Error.  Unable to allocate memory for all job records.\n");
                return -1;
	    }
    swf_job_arr = (swf_job_trace_t *)malloc(sizeof(swf_job_trace_t) * nrecs);
    if (!swf_job_arr) {
                printf("Error.  Unable to allocate memory for all job records.\n");
                return -1;
    }
    job_trace_head = &job_arr[0];
    job_ptr = &job_arr[0];
    while (fgets(line, sizeof(line), file)) {
        /* note that fgets don't strip the terminating \n, checking its
           presence would allow to handle lines longer that sizeof(line) */
        //printf("%s", line);
        if(strncmp(line, "#", 1) == 0)
                continue;
        if(strncmp(line, ";", 1) == 0)
                continue;
	p = strtok(line, " \t");
        i=0;
        while(p!=NULL){
	    if(i==0) {
		job_arr[idx].job_id = ++job_index; 
		swf_job_arr[idx].job_num = job_arr[idx].job_id;
	//	printf("Index is: %d", job_index); 
	    }   
            if(i==1) {
	//	printf("%s", p);
		job_arr[idx].submit = 100 + atol(p);
		swf_job_arr[idx].sub_time = job_arr[idx].submit;
	    }  // why submit cannot start from 0? 
            if(i==2) {
          //      printf("%s", p);
                swf_job_arr[idx].wait_time = atol(p);
            }
            if(i==3) {
	//	printf("%s", p);
		uint32_t n;
        //        printf("End time: %s\n", p);
                sscanf(p, "%"SCNu32, &n);
                job_arr[idx].duration = (uint32_t) n; 
		swf_job_arr[idx].run_time = job_arr[idx].duration;
	    }
	   // if (i == 4)
	//	swf_job_arr[idx].alloc_cpus = -1;//atoi(p); ater adapting to CPUS_PER_NODE this would differ
            if (i == 5)
                swf_job_arr[idx].avg_cpu_time = atol(p); //this too
            if (i == 6)
                swf_job_arr[idx].mem = atol(p); //actually not simulated
            if (i == 4) { //use req cpus (4) instead of allocated (7)
		swf_job_arr[idx].alloc_cpus = -1;//atoi(p); ater adapting to CPUS_PER_NODE this would differ
		//printf("%s", p);
		job_arr[idx].tasks = ceil((float)atol(p)/CPUS_PER_NODE); 
		swf_job_arr[idx].req_cpus = job_arr[idx].tasks * CPUS_PER_NODE; //round to full node
	//	printf("Number of tasks %d, number of CPUs %d", job_arr[idx].tasks, swf_job_arr[idx].req_cpus);
	    }
            if(i==8) {
		long wclim = atol(p);
		wclim = ceil((double)wclim / 60.0f);
		if (wclim >= INT_MAX) {
			job_arr[idx].wclimit = INT_MAX;
			printf("wclimit too high!\n");
		}
		else 
			job_arr[idx].wclimit = wclim;
		swf_job_arr[idx].req_time = job_arr[idx].wclimit * 60; //round to minutes
	    }
	    if (i == 9)
                swf_job_arr[idx].req_mem = atol(p);
            if (i == 10) //i copy the status, but we only support completed jobs
                swf_job_arr[idx].status = atol(p);
            if (i == 11) //not using this, single user
                swf_job_arr[idx].uid = atol(p);
	    if (i == 12)
                swf_job_arr[idx].gid = atol(p);
            if (i == 13)
                swf_job_arr[idx].app_num = atol(p);
            if (i == 14)
                swf_job_arr[idx].queue_num = atol(p);
	    if (i == 15)
                swf_job_arr[idx].part_num = atol(p);
            if (i == 16)
                swf_job_arr[idx].prev_job_num = atol(p);
            if (i == 17)
                swf_job_arr[idx].think_time_prev_job = atol(p);
            //if(i==11) { printf("%s", p); strcpy(job_arr[idx].username, p); }   
            //if(i==12) { printf("%s", p); strcpy(job_arr[idx].account, p); }   
            //if(i==15) { printf("%s", p); strcpy(job_arr[idx].partition, p); }   
          //  printf(" %s\n", p);
            p = strtok(NULL," ");
            i++;
        }
        // assuming pure MPI application; for threaded one we will have to do it differently.
        //job_arr[idx].cpus_per_task = 1;
        //if(job_arr[idx].tasks < CPUS_PER_NODE) job_arr[idx].tasks_per_node = job_arr[idx].tasks; 
        //else job_arr[idx].tasks_per_node = CPUS_PER_NODE;

        if(job_arr[idx].tasks == 0) { job_arr[idx].tasks=1; }
        //assuming one task per node, and as many threads as CPUs per node
        job_arr[idx].cpus_per_task = CPUS_PER_NODE;
        job_arr[idx].tasks_per_node = 1;

        // for now keep username, partition and account constant.
        strcpy(job_arr[idx].username, "tester");
        strcpy(job_arr[idx].partition, "normal");
        strcpy(job_arr[idx].account, "1000");
        //strcpy(job_trace->manifest_filename, "|\0");
        //job_trace->manifest=NULL;
	if (job_arr[idx].wclimit <= 0) {
	  printf("%ld: wclimit <= 0!\n", job_arr[idx].job_id);
	  job_arr[idx].wclimit = job_arr[idx].duration;
	  swf_job_arr[idx].req_time = job_arr[idx].wclimit * 60; 
	}
        if((double)(job_arr[idx].wclimit * 60) < job_arr[idx].duration ){
	  printf("%ld: duration greater than wclimit! (%d > %ld)\n", job_arr[idx].job_id, job_arr[idx].duration, job_arr[idx].wclimit * 60);
	  job_arr[idx].duration = job_arr[idx].wclimit * 60;
	  swf_job_arr[idx].run_time = job_arr[idx].duration;
    	}
	if(job_arr[idx].duration <= 0){
		printf("%ld: duration <= 0!\n", job_arr[idx].job_id);
		job_arr[idx].duration = 1;
		swf_job_arr[idx].run_time = 1;
	}
	
	idx++;
        counter++;
    }
    /* may check feof here to make a difference between eof and io failure -- network
       timeout for instance */

    fclose(file);

    FILE *swf_fp = fopen("swf_trace","w"); 
    int trace_file, written;
    char *ofile         = NULL;
    if (!ofile) ofile = (char*)DEFAULT_OFILE;
    /* open trace file: */
    if ((trace_file = open(ofile, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR |
                                                S_IRGRP | S_IROTH)) < 0) {
       printf("Error opening trace file %s\n", ofile);
       return -1;
    }
    job_ptr = job_trace_head;
    int j=0;
    int nrecs_limit = counter;
    while(j<nrecs_limit){
	written = write(trace_file, &job_arr[j], sizeof(job_trace_t));
        if (written <= 0) {
                printf("Error! Zero bytes written.\n");
        	++errs;
        }
	fprintf(swf_fp, "%d\t%ld\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",swf_job_arr[j].job_num,
				swf_job_arr[j].sub_time,
				swf_job_arr[j].wait_time,
				swf_job_arr[j].run_time,
				swf_job_arr[j].alloc_cpus,
				swf_job_arr[j].avg_cpu_time,
				swf_job_arr[j].mem,
                                swf_job_arr[j].req_cpus,
                                swf_job_arr[j].req_time,
                                swf_job_arr[j].req_mem,
                                swf_job_arr[j].status,
                                swf_job_arr[j].uid,
                                swf_job_arr[j].gid,
                                swf_job_arr[j].app_num,
                                swf_job_arr[j].queue_num,
				swf_job_arr[j].part_num,
				swf_job_arr[j].prev_job_num,
				swf_job_arr[j].think_time_prev_job);
        //job_ptr = job_ptr->next;
        j++;
    }
    fclose(swf_fp);
    close(trace_file);
    free(job_arr);
    return 0;
}


