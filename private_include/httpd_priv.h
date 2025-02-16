#include <stdio.h>
#include "../http_parse.h"

typedef ssize_t (*read_callback_t)(char *buffer, size_t offset, void *user_data);

typedef struct http_parser_handle {
    http_req_t *http_req;
    read_callback_t read_callback;
    size_t parse_offset;
} http_parser_handle_t;


