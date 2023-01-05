
#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdbool.h>
#include <glibwrap.h>
#include <dataset.h>
#include <arpa/inet.h>

/* TEMPORARY PARAMETERS */
extern int M;
extern bool geos_prepare;
extern int servers;

extern int jcondcount;
extern char *jconditions[100];
extern GList *joinplan;
extern void *lastjoinpredicate;

/* global parameters */
extern char *verbose;
extern bool printed_nonverbose_header;

/* ACTIONS */
bool system_connect(const char *host, int port);
bool run_load_action(const char *shpfile, const char *datasetname, bool geos_prepare, int servers, bool simulate_load);
bool run_distr_join_action(GList *joinplan, bool only_plan, void (*plan_action)(GList *, bool));
void djoin_execute(GList *joinplan, bool only_plan);
void clean_intermediates();
void clean_local_datasets();

/* Auxiliary */
int startparser(char *argv);
void run_system_cmd(char *cmd);
void add_new_dataset(dataset *ds);
 
#endif

