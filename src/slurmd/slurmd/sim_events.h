#ifdef SLURM_SIMULATOR
/*
 ** Definitions for simulation mode
 ** */

typedef struct simulator_event_info{
	int job_id;
	int duration;
	struct simulator_event_info *next;
	struct simulator_event_info *prev;
} simulator_event_info_t;

typedef struct simulator_event{
    int job_id;
    int type;
    time_t when;
    char *nodelist;
	int pack_components;
	simulator_event_info_t *event_info_ptr;
    volatile struct simulator_event *next;
} simulator_event_t;
#endif
