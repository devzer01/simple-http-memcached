#include <stdio.h>

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <string.h>

#include <libmemcached-1.0/memcached.h>

#define PORT 8888

memcached_st *memc;

memcached_st* connect_to_memcached() {
    const char *config_string = "--SERVER=127.0.0.1";
    memcached_st *memc = memcached(config_string, strlen(config_string));
    return memc;
}

void disconnect_from_memcached(memcached_st* memc) {
    memcached_free(memc);
}

void set_kv(memcached_st* memc, char *key, char *value) {

    memcached_return_t rc= memcached_set(memc, key, strlen(key), value, strlen(value), (time_t)0, (uint32_t)0);

    if (rc != MEMCACHED_SUCCESS)
    {
        printf("error creating memcached value");
    }
}

char* get_kv(memcached_st* memc, const char *key) {
    size_t *value_length = NULL;
    uint32_t *flags = NULL;
    memcached_return_t *error = NULL;
    return memcached_get(memc, key, sizeof(key), value_length, flags, error);
}

int answer_to_connection (void *cls, struct MHD_Connection *connection,
                          const char *url,
                          const char *method, const char *version,
                          const char *upload_data,
                          size_t *upload_data_size, void **con_cls) {

    char *value = get_kv(memc, "root");

    char *page;
    sprintf(page, "<html><body>Hello, browser! %s</body></html>", value);
    struct MHD_Response *response;
    int ret;

    response = MHD_create_response_from_buffer (strlen (page),
                                                (void*) page, MHD_RESPMEM_PERSISTENT);

    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);

    return ret;
}

int main() {
    printf("Hello, World!\n");

    memc = connect_to_memcached();
    set_kv(memc, "root", "hello world");

    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL,
                               (MHD_AccessHandlerCallback) &answer_to_connection, NULL, MHD_OPTION_END);
    if (NULL == daemon) return 1;

    getchar ();

    MHD_stop_daemon (daemon);

    disconnect_from_memcached(memc);
    return 0;
}
