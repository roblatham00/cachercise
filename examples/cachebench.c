/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <stdio.h>
#include <stdlib.h>
#include <margo.h>
#include <assert.h>
#include <unistd.h>

#include <mpi.h>
#include <ssg.h>
#include <json-c/json.h>
#include "json-macros.h"

#include <cachercise/cachercise-admin.h>
#include <cachercise/cachercise-client.h>
#include <cachercise/cachercise-cache.h>

#define FATAL(...) \
    do { \
        margo_critical(__VA_ARGS__); \
        exit(-1); \
    } while(0)


struct options {
    char group_file[256];
    char json_file[256];
    char output_file[256];
};

void usage()
{
    fprintf(stderr, "Usage: cachebench -g <group_file> -j <json_config>\n");
    return;

}

static int parse_json(const char* json_file, struct json_object** json_cfg)
{
    struct json_tokener*    tokener;
    enum json_tokener_error jerr;
    char*                   json_cfg_str = NULL;
    FILE*                   f;
    long                    fsize;
    int                     nranks;
    struct json_object*     val;

    /* open json file */
    f = fopen(json_file, "r");
    if (!f) {
        perror("fopen");
        fprintf(stderr, "Error: could not open json file %s\n", json_file);
        return (-1);
    }

    /* check size */
    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    /* allocate space to hold contents and read it in */
    json_cfg_str = malloc(fsize + 1);
    if (!json_cfg_str) {
        perror("malloc");
        return (-1);
    }
    fread(json_cfg_str, 1, fsize, f);
    fclose(f);
    json_cfg_str[fsize] = 0;

    /* parse json */
    tokener = json_tokener_new();
    *json_cfg
        = json_tokener_parse_ex(tokener, json_cfg_str, strlen(json_cfg_str));
    if (!(*json_cfg)) {
        jerr = json_tokener_get_error(tokener);
        fprintf(stderr, "JSON parse error: %s", json_tokener_error_desc(jerr));
        json_tokener_free(tokener);
        free(json_cfg_str);
        return -1;
    }
    json_tokener_free(tokener);
    free(json_cfg_str);

    /* validate input params or fill in defaults */
    MPI_Comm_size(MPI_COMM_WORLD, &nranks);
    CONFIG_OVERRIDE_INTEGER(*json_cfg, "nranks", nranks, 1);

    /* set defaults if not present */
    CONFIG_HAS_OR_CREATE(*json_cfg, int, "items_per_process", 100, val);

    return (0);
}

static int parse_args(int argc,
        char **argv,
        struct options * opts,
        struct json_object **json_cfg)
{
    int opt, ret;
    opts->json_file[0] = 0;
    opts->group_file[0] = 0;

    while ( (opt = getopt(argc, argv, "g:j:")) != -1) {
        switch (opt) {
            case 'g':
                ret = sscanf(optarg, "%s", opts->group_file);
                if (ret != 1) return -1;
                break;
            case 'j':
                ret = sscanf(optarg, "%s", opts->json_file);
                if (ret != 1) return -1;
                break;
            default:
                return -1;
        }
    }
    if (opts->json_file[0]) return (parse_json(opts->json_file, json_cfg));
    return 0;
}

int dump_json(margo_instance_id mid, cachercise_cache_handle_t cachercise_rh, struct json_object * json_cfg)
{
    struct json_tokener*    tokener;
    enum json_tokener_error jerr;
    char* cli_cfg_str = NULL;
    struct json_object* margo_config;

    /* retrieve local margo configuration */
    cli_cfg_str = margo_get_config(mid);

    /* parse margo config and injected into the benchmark config */
    tokener = json_tokener_new();
    margo_config
	= json_tokener_parse_ex(tokener, cli_cfg_str, strlen(cli_cfg_str));
    if (!margo_config) {
	jerr = json_tokener_get_error(tokener);
	fprintf(stderr, "JSON parse error: %s",
		json_tokener_error_desc(jerr));
	json_tokener_free(tokener);
	return -1;
    }
    json_tokener_free(tokener);
    /* delete existing margo object, if present */
    json_object_object_del(json_cfg, "margo");
    /* add new one, derived at run time */
    json_object_object_add(json_cfg, "margo", margo_config);

    printf("\"quintain-benchmark\" : %s\n",
	    json_object_to_json_string_ext(
		json_cfg,
		JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_NOSLASHESCAPE));
    return 0;
}

int main(int argc, char** argv)
{

    int ret;
    int nprocs, rank, nproviders;
    struct options opts;
    char proto[64] = {0};
    margo_instance_id mid;
    struct margo_init_info   mii          = {0};
    struct json_object* margo_config = NULL;
    struct json_object* json_cfg;
    ssg_group_id_t gid;
    ssg_member_id_t svr_id;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    char * svr_addr_str = NULL;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    ret = parse_args(argc, argv, &opts, &json_cfg);
    if (ret < 0){
        usage();
        exit(EXIT_FAILURE);
    }

    ret = ssg_get_group_transport_from_file(opts.group_file, proto, 64);
    margo_config = json_object_object_get(json_cfg, "margo");
    if (margo_config)
        mii.json_config = json_object_to_json_string_ext(
                margo_config, JSON_C_TO_STRING_PLAIN);
    mid = margo_init_ext(proto, MARGO_CLIENT_MODE, &mii);
    if (!mid) {
        fprintf(stderr, "Error: failed to initialize margo with %s protocol.\n",
                proto);
        ret = -1;
        goto err_mpi_cleanup;
    }
    ret = ssg_init();
    if (ret != SSG_SUCCESS) {
        fprintf(stderr, "Error: ssg_init(): %s.\n", ssg_strerror(ret));
        goto err_margo_cleanup;
    }

    /* load ssg group information */
    nproviders = 1;
    ret        = ssg_group_id_load(opts.group_file, &nproviders, &gid);
    if (ret != SSG_SUCCESS) {
        fprintf(stderr, "Error: ssg_group_id_load(): %s.\n", ssg_strerror(ret));
        goto err_ssg_cleanup;
    }

    /* refresh view of servers */
    ret = ssg_group_refresh(mid, gid);
    if (ret != SSG_SUCCESS) {
        fprintf(stderr, "Error: ssg_group_refresh(): %s.\n", ssg_strerror(ret));
        goto err_ssg_cleanup;
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0)
        printf("# SSG group refreshed; proceeding with benchmark.\n");

    /* get addr for rank 0 in ssg group */
    ret = ssg_get_group_member_id_from_rank(gid, 0, &svr_id);
    if (ret != SSG_SUCCESS) {
        fprintf(stderr, "Error: ssg_group_member_id_from_rank(): %s.\n",
                ssg_strerror(ret));
        goto err_ssg_cleanup;
    }

    ret = ssg_get_group_member_addr_str(gid, svr_id, &svr_addr_str);
    if (ret != SSG_SUCCESS) {
        fprintf(stderr, "Error: ssg_get_group_member_addr_str(): %s.\n",
                ssg_strerror(ret));
        goto err_ssg_cleanup;
    }

    ret = margo_addr_lookup(mid, svr_addr_str, &svr_addr);
    if (ret != HG_SUCCESS) {
        fprintf(stderr, "Error: margo_addr_lookup()\n");
        goto err_ssg_cleanup;
    }

    margo_info(mid,"Initializing admin");
    cachercise_admin_t admin;
    ret = cachercise_admin_init(mid, &admin);
    if(ret != CACHERCISE_SUCCESS) {
        FATAL(mid,"cachercise_admin_init failed (ret = %d)", ret);
    }

    margo_info(mid,"Creating cache");
    cachercise_cache_id_t cache_id;
    if (rank == 0) {

        /* TODO: can we get the provider id programatically? */
        ret = cachercise_create_cache(admin, svr_addr, 1, NULL,
                "dummy", "{}", &cache_id);
        if(ret != CACHERCISE_SUCCESS) {
            FATAL(mid,"cachercise_create_cache failed (ret = %d)", ret);
        }
    }
    MPI_Bcast(&cache_id, sizeof(cache_id), MPI_BYTE, 0, MPI_COMM_WORLD);


    cachercise_client_t cachercise_clt;
    cachercise_cache_handle_t cachercise_rh;

    margo_info(mid, "Creating CACHERCISE client");
    ret = cachercise_client_init(mid, &cachercise_clt);
    if(ret != CACHERCISE_SUCCESS) {
        FATAL(mid,"cachercise_client_init failed (ret = %d)", ret);
    }

    margo_info(mid, "Creating cache handle");
    // again, need provider id
    ret = cachercise_cache_handle_create(
            cachercise_clt, svr_addr, 1,
            cache_id, &cachercise_rh);
    if(ret != CACHERCISE_SUCCESS) {
        FATAL(mid,"cachercise_cache_handle_create failed (ret = %d)", ret);
    }

    int nr_items = json_object_get_int(
            json_object_object_get(json_cfg, "items_per_process"));

    double duration = MPI_Wtime();
    int i;
    for (i=0; i< nr_items; i++ ) {
        int64_t value=i*nprocs+rank+100;
        ret = cachercise_write(cachercise_rh, &value, sizeof(value), i*nprocs+rank);
    }
    duration = MPI_Wtime() - duration;

    double min_duration, max_duration, sum_duration;
    MPI_Reduce(&duration, &max_duration, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&duration, &min_duration, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&duration, &sum_duration, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        int64_t total = nr_items*nprocs;
        printf("%d procs %ld updates in %f %f %f seconds: %f %f %f updates/sec\n",
                nprocs, total,
                sum_duration/nprocs, min_duration, max_duration,
                total/(sum_duration/nprocs), total/max_duration, total/min_duration);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    int64_t j, nerrors=0;
    if (rank == 0) {
        int64_t compare=-9999;
        for (j=0; j < i*nprocs; j++) {
            ret = cachercise_read(cachercise_rh, &compare, sizeof(compare), j );
            if (compare != j+100) {
                nerrors++;
                printf("expected %ld got %ld\n", j+100, compare);
            }
            if (nerrors > 5) break;
        }
    }

    if (rank == 0)
        dump_json(mid, cachercise_rh, json_cfg);

    margo_info(mid, "Releasing cache handle");
    ret = cachercise_cache_handle_release(cachercise_rh);
    if(ret != CACHERCISE_SUCCESS) {
        FATAL(mid,"cachercise_cache_handle_release failed (ret = %d)", ret);
    }

    margo_info(mid, "Finalizing client");
    ret = cachercise_client_finalize(cachercise_clt);
    if(ret != CACHERCISE_SUCCESS) {
        FATAL(mid,"cachercise_client_finalize failed (ret = %d)", ret);
    }

    margo_info(mid,"Finalizing admin");
    ret = cachercise_admin_finalize(admin);
    if(ret != CACHERCISE_SUCCESS) {
        FATAL(mid,"cachercise_admin_finalize failed (ret = %d)", ret);
    }


err_ssg_cleanup:
    if (svr_addr_str) free(svr_addr_str);
    ssg_finalize();
err_margo_cleanup:
    if (svr_addr != HG_ADDR_NULL) margo_addr_free(mid, svr_addr);
    margo_finalize(mid);
err_mpi_cleanup:
    if (json_cfg) json_object_put(json_cfg);
    if (ret != 0)
        MPI_Abort(MPI_COMM_WORLD, -1);
    else
        MPI_Finalize();

    return ret;
}


