/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <stdio.h>
#include <margo.h>
#include <alpha/alpha-server.h>
#include <alpha/alpha-admin.h>
#include <alpha/alpha-client.h>
#include <alpha/alpha-resource.h>
#include "munit/munit.h"

struct test_context {
    margo_instance_id   mid;
    hg_addr_t           addr;
    alpha_admin_t       admin;
    alpha_resource_id_t id;
};

static const char* token = "ABCDEFGH";
static const uint16_t provider_id = 42;
static const char* backend_config = "{ \"foo\" : \"bar\" }";

static void* test_context_setup(const MunitParameter params[], void* user_data)
{
    (void) params;
    (void) user_data;
    alpha_return_t      ret;
    margo_instance_id   mid;
    hg_addr_t           addr;
    alpha_admin_t       admin;
    alpha_resource_id_t id;
    // create margo instance
    mid = margo_init("na+sm", MARGO_SERVER_MODE, 0, 0);
    munit_assert_not_null(mid);
    // get address of current process
    hg_return_t hret = margo_addr_self(mid, &addr);
    munit_assert_int(hret, ==, HG_SUCCESS);
    // register alpha provider
    struct alpha_provider_args args = ALPHA_PROVIDER_ARGS_INIT;
    args.token = token;
    ret = alpha_provider_register(
            mid, provider_id, &args,
            ALPHA_PROVIDER_IGNORE);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // create an admin
    ret = alpha_admin_init(mid, &admin);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // create a resource using the admin
    ret = alpha_create_resource(admin, addr,
            provider_id, token, "dummy", backend_config, &id);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // create test context
    struct test_context* context = (struct test_context*)calloc(1, sizeof(*context));
    munit_assert_not_null(context);
    context->mid   = mid;
    context->addr  = addr;
    context->admin = admin;
    context->id    = id;
    return context;
}

static void test_context_tear_down(void* fixture)
{
    alpha_return_t ret;
    struct test_context* context = (struct test_context*)fixture;
    // destroy the resource
    ret = alpha_destroy_resource(context->admin,
            context->addr, provider_id, token, context->id);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // free the admin
    ret = alpha_admin_finalize(context->admin);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // free address
    margo_addr_free(context->mid, context->addr);
    // we are not checking the return value of the above function with
    // munit because we need margo_finalize to be called no matter what.
    margo_finalize(context->mid);
}

static MunitResult test_client(const MunitParameter params[], void* data)
{
    (void)params;
    (void)data;
    struct test_context* context = (struct test_context*)data;
    alpha_client_t client;
    alpha_return_t ret;
    // test that we can create a client object
    ret = alpha_client_init(context->mid, &client);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // test that we can free the client object
    ret = alpha_client_finalize(client);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);

    return MUNIT_OK;
}

static MunitResult test_resource(const MunitParameter params[], void* data)
{
    (void)params;
    (void)data;
    struct test_context* context = (struct test_context*)data;
    alpha_client_t client;
    alpha_resource_handle_t rh;
    alpha_return_t ret;
    // test that we can create a client object
    ret = alpha_client_init(context->mid, &client);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // test that we can create a resource handle
    ret = alpha_resource_handle_create(client,
            context->addr, provider_id, context->id, &rh);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // test that we can increase the ref count
    ret = alpha_resource_handle_ref_incr(rh);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // test that we can destroy the resource handle
    ret = alpha_resource_handle_release(rh);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // ... and a second time because of the increase ref 
    ret = alpha_resource_handle_release(rh);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // test that we can free the client object
    ret = alpha_client_finalize(client);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);

    return MUNIT_OK;
}


static MunitResult test_hello(const MunitParameter params[], void* data)
{
    (void)params;
    (void)data;
    struct test_context* context = (struct test_context*)data;
    alpha_client_t client;
    alpha_resource_handle_t rh;
    alpha_return_t ret;
    // test that we can create a client object
    ret = alpha_client_init(context->mid, &client);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // test that we can create a resource handle
    ret = alpha_resource_handle_create(client,
            context->addr, provider_id, context->id, &rh);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // test that we can send a hello RPC to the resource
    ret = alpha_say_hello(rh);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // test that we can destroy the resource handle
    ret = alpha_resource_handle_release(rh);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // test that we can free the client object
    ret = alpha_client_finalize(client);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);

    return MUNIT_OK;
}

static MunitResult test_sum(const MunitParameter params[], void* data)
{
    (void)params;
    (void)data;
    struct test_context* context = (struct test_context*)data;
    alpha_client_t client;
    alpha_resource_handle_t rh;
    alpha_return_t ret;
    // test that we can create a client object
    ret = alpha_client_init(context->mid, &client);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // test that we can create a resource handle
    ret = alpha_resource_handle_create(client,
            context->addr, provider_id, context->id, &rh);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // test that we can send a sum RPC to the resource
    int32_t result = 0;
    ret = alpha_compute_sum(rh, 45, 55, &result);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    munit_assert_int(result, ==, 100);
    // test that we can destroy the resource handle
    ret = alpha_resource_handle_release(rh);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // test that we can free the client object
    ret = alpha_client_finalize(client);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);

    return MUNIT_OK;
}

static MunitResult test_invalid(const MunitParameter params[], void* data)
{
    (void)params;
    (void)data;
    struct test_context* context = (struct test_context*)data;
    alpha_client_t client;
    alpha_resource_handle_t rh1, rh2;
    alpha_resource_id_t invalid_id;
    alpha_return_t ret;
    // test that we can create a client object
    ret = alpha_client_init(context->mid, &client);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // create a resource handle for a wrong resource id
    ret = alpha_resource_handle_create(client,
            context->addr, provider_id, invalid_id, &rh1);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // create a resource handle for a wrong provider id
    ret = alpha_resource_handle_create(client,
            context->addr, provider_id + 1, context->id, &rh2);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // test sending to the invalid resource id
    int32_t result;
    ret = alpha_compute_sum(rh1, 45, 55, &result);
    munit_assert_int(ret, ==, ALPHA_ERR_INVALID_RESOURCE);
    // test sending to the invalid provider id
    ret = alpha_compute_sum(rh2, 45, 55, &result);
    munit_assert_int(ret, ==, ALPHA_ERR_FROM_MERCURY);
    // test that we can destroy the resource handle
    ret = alpha_resource_handle_release(rh1);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // test that we can destroy the resource handle
    ret = alpha_resource_handle_release(rh2);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);
    // test that we can free the client object
    ret = alpha_client_finalize(client);
    munit_assert_int(ret, ==, ALPHA_SUCCESS);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char*) "/client",   test_client,   test_context_setup, test_context_tear_down, MUNIT_TEST_OPTION_NONE, NULL },
    { (char*) "/resource", test_resource, test_context_setup, test_context_tear_down, MUNIT_TEST_OPTION_NONE, NULL },
    { (char*) "/hello",    test_hello,    test_context_setup, test_context_tear_down, MUNIT_TEST_OPTION_NONE, NULL },
    { (char*) "/sum",      test_sum,      test_context_setup, test_context_tear_down, MUNIT_TEST_OPTION_NONE, NULL },
    { (char*) "/invalid",  test_invalid,  test_context_setup, test_context_tear_down, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = { 
    (char*) "/alpha/admin", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, (void*) "alpha", argc, argv);
}
