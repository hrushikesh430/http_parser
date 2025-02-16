#include "../../http_parse.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void) {
    http_req_t http_req;
    http_req.data = "GET /hello/world?first=king&name=John&sun=moon HTTP/1.1\r\nHost: localhost:8080\r\nHeader: value\r\nHeader2: value2\r\n\r\nHello World, this is a test\r\n\r\n"; // split this into multiple parts
    http_req.data_len = strlen(http_req.data);

    void *parser_handle = parser_init(&http_req);
    int length;
    if (parse_new_req(parser_handle) == 0) {
        char *buf;
        int buf_len;

        // Parse URI
        if (parse_get_uri(parser_handle, &buf, &buf_len) == 0) {
            printf("URI: %.*s\n", buf_len, buf);
        }
        else {
            printf("Failed to parse URI\n");
        }
        

        // Parse query parameters
        if (parse_uri_query_param(parser_handle, "first", &buf, &length) == 0) {
            printf("first: %.*s\n", length, buf);
        }

        // Parse headers
        if (parse_get_hdr_value(parser_handle, "any", &buf, &length) == 0) {
            printf("Header: %.*s\n", length, buf);
        }
        if (parse_get_hdr_value(parser_handle, "Header2", &buf, &length) == 0) {
            printf("Header2: %.*s\n", length, buf);
        }
        if (parse_get_hdr_value(parser_handle, "Header", &buf, &length) == 0) {
            printf("Header: %.*s\n", length, buf);
        }

        // Parse payload
        char *payload;
        int payload_len;
        if (parse_req_payload(parser_handle, &payload, &payload_len) == 0) {
            printf("Payload: %.*s\n", payload_len, payload);
        }
    }

    return 0; 
}