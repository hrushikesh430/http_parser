#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "private_include/httpd_priv.h"

#define HTTP_PARSER_READ_BUFFER_SIZE 128

ssize_t http_read_callback(char *buffer, size_t offset, void *user_data) {
    printf("http_read_callback: offset: %zu\n", offset);
    http_parser_handle_t *parser_handle = (http_parser_handle_t *)user_data;
    if (parser_handle == NULL || parser_handle->http_req == NULL) {
        return -1; // Invalid parser handle
    }

    // Check if the offset is within the current data length
    if (offset >= parser_handle->http_req->data_len) {
        return 0; // No more data to read
    }

    // Calculate the number of bytes to read
    size_t bytes_to_read = HTTP_PARSER_READ_BUFFER_SIZE;
    if (offset + HTTP_PARSER_READ_BUFFER_SIZE > parser_handle->http_req->data_len) {
        bytes_to_read = parser_handle->http_req->data_len - offset;
    }

    // Copy the data from the offset to the buffer
    // memcpy(buffer, parser_handle->http_req->data + offset, bytes_to_read);

    // Update the parse offset
    parser_handle->parse_offset = offset + bytes_to_read;

    return bytes_to_read;
}

void *parser_init(http_req_t *http_req) {
    http_parser_handle_t *parser_handle = (http_parser_handle_t *)malloc(sizeof(http_parser_handle_t));
    if (parser_handle == NULL) {
        return NULL;
    }

    parser_handle->http_req = http_req;
    parser_handle->read_callback = http_read_callback;
    parser_handle->parse_offset = 0;
    return parser_handle;
}

int parse_new_req(void *http_parser_handle) {
    http_parser_handle_t *parser_handle = (http_parser_handle_t *)http_parser_handle;
    
    return 0;
}

int parse_get_uri(void *http_parser_handle, char **buf, int *buf_len) {
    http_parser_handle_t *parser_handle = (http_parser_handle_t *)http_parser_handle;

    // Check if the parser_handle or http_req is NULL
    if (parser_handle == NULL || parser_handle->http_req == NULL) {
        return -1; // Invalid parser handle
    }

    // Attempt to find the URI, calling read_callback if necessary
    char *method_end = NULL;
    while (1) {
        // Traverse the data buffer up to the current parse offset
        method_end = memchr(parser_handle->http_req->data, ' ', parser_handle->parse_offset);
        if (method_end != NULL) {
            break; // Found the space after the method
        }

        // Call read_callback to attempt to read more data
        ssize_t bytes_read = parser_handle->read_callback(parser_handle->http_req->data, parser_handle->parse_offset, parser_handle);
        if (bytes_read <= 0) {
            printf("parse_get_uri: No more data to read or error\n");
            return -1; // No more data to read or error
        }
    }

    char *uri_start = method_end + 1; // Start after the method
    char *uri_end = NULL;
    while (1) {
        uri_end = memchr(uri_start, ' ', parser_handle->http_req->data_len - (uri_start - parser_handle->http_req->data));
        if (uri_end != NULL) {
            break; // Found the space after the URI
        }

        // Call read_callback to attempt to read more data
        ssize_t bytes_read = parser_handle->read_callback(parser_handle->http_req->data, parser_handle->parse_offset, parser_handle);
        if (bytes_read <= 0) {
            printf("parse_get_uri: No more data to read or error\n");
            return -1; // No more data to read or error
        }
    }
    while (1) {
        // Check for '?' in the URI and adjust uri_end if present
        char *query_start = strchr(uri_start, '?');
        if (query_start != NULL && query_start < uri_end) {
            uri_end = query_start; // Exclude '?' and everything after it
            break;
        }
        // Call read_callback to attempt to read more data
        ssize_t bytes_read = parser_handle->read_callback(parser_handle->http_req->data, parser_handle->parse_offset, parser_handle);
        if (bytes_read <= 0) {
            printf("parse_get_uri: No more data to read or error\n");
            return -1; // No more data to read or error
        }
    }

    size_t uri_length = uri_end - uri_start; // Length of the URI

    *buf = uri_start;
    *buf_len = uri_length; // Update the length to return

    // Update the parse offset to the end of the URI
    parser_handle->parse_offset = uri_end - parser_handle->http_req->data;

    return 0;
}

int parse_get_hdr_value(void *http_parser_handle, char *key, char **value, int *length) {
    http_parser_handle_t *parser_handle = (http_parser_handle_t *)http_parser_handle;

    // Check if the request is complete
    if (parser_handle->http_req->data_len == 0 || parser_handle->http_req->data == NULL) {
        return -1; // No data to read
    }

    size_t offset = 0; // Local offset to track position

    // Find the start of the headers by searching for the first newline character
    char *header_start = strstr(parser_handle->http_req->data, "\r\n");
    if (header_start != NULL) {
        offset = header_start - parser_handle->http_req->data + 2; // Move offset to the start of the headers
    }

    // Loop to extract key-value pairs, limited to parser_offset
    while (offset < parser_handle->parse_offset) {
        // Extract the key
        char *key_start = parser_handle->http_req->data + offset;
        char *key_end = strchr(key_start, ':');
        if (key_end == NULL || key_end >= parser_handle->http_req->data + parser_handle->parse_offset) {
            break; // No colon found within the limit, or invalid request
        }
        size_t key_length = key_end - key_start;

        // Move past the colon and any whitespace
        char *value_start = key_end + 1;
        while (*value_start == ' ' && value_start < parser_handle->http_req->data + parser_handle->parse_offset) value_start++; // Skip any spaces after the colon

        // Extract the value
        char *value_end = strchr(value_start, '\r');
        if (value_end == NULL || value_end >= parser_handle->http_req->data + parser_handle->parse_offset) {
            break; // No newline found within the limit, or invalid request
        }
        size_t value_length = value_end - value_start;

        // Check if the key matches the given key
        if (strncmp(key_start, key, key_length) == 0 && key[key_length] == '\0') {
            *value = value_start;
            *length = value_length; // Update the length to return

            // Update the parse offset to the end of the header
            parser_handle->parse_offset = value_end - parser_handle->http_req->data + 2;

            return 0; // Successfully parsed key-value pair
        }

        offset = value_end - parser_handle->http_req->data + 2; // Move offset to the start of the next header key after \r\n
    }

    // If key is not found, invoke the read callback and retry
    ssize_t read_result;
    do {
        read_result = parser_handle->read_callback(parser_handle->http_req->data, parser_handle->parse_offset, parser_handle);
        if (read_result > 0) {
            return parse_get_hdr_value(http_parser_handle, key, value, length); // Retry parsing after reading more data
        }
    } while (read_result > 0);

    return -1; // Key not found
}

int parse_uri_query_param(void *http_parser_handle, char *key, char **value, int *length) {
    http_parser_handle_t *parser_handle = (http_parser_handle_t *)http_parser_handle;

    // Loop to handle reading more data if necessary
    while (1) {
        // Start searching from the beginning of the data
        char *query_start = strchr(parser_handle->http_req->data, '?');
        if (query_start == NULL || query_start >= parser_handle->http_req->data + parser_handle->parse_offset) {
            // If not found, invoke the read callback
            ssize_t read_result = parser_handle->read_callback(parser_handle->http_req->data, parser_handle->parse_offset, parser_handle);
            if (read_result <= 0) {
                return -1; // Stop if no more data is read
            }
            continue; // Retry after reading more data
        }
        
        char *query_end = strchr(query_start, ' ');
        if (query_end == NULL || query_end >= parser_handle->http_req->data + parser_handle->parse_offset) {
            // If not found, invoke the read callback
            ssize_t read_result = parser_handle->read_callback(parser_handle->http_req->data, parser_handle->parse_offset, parser_handle);
            if (read_result <= 0) {
                return -1; // Stop if no more data is read
            }
            continue; // Retry after reading more data
        }

        // Find the key in the query string within the current parse offset
        char *key_start = strstr(query_start, key);
        while (key_start != NULL && key_start < parser_handle->http_req->data + parser_handle->parse_offset) {
            // Move the pointer to the start of the value
            char *value_start = key_start + strlen(key) + 1; // +1 for the '='
            char *value_end = strchr(value_start, '&'); // Find the end of the value
            if (value_end == NULL || value_end > query_end) {
                value_end = query_end; // If no '&', the end is the query_end
            }

            size_t value_length = value_end - value_start;
            *value = strndup(value_start, value_length);
            if (*value == NULL) {
                return -1; // Memory allocation failed
            }

            // Update the length to return
            *length = value_length;

            // Update the parse offset to the end of the query parameter
            parser_handle->parse_offset = value_end - parser_handle->http_req->data;

            return 0; // Successfully parsed the key-value pair
        }

        // If key is not found, invoke the read callback
        ssize_t read_result = parser_handle->read_callback(parser_handle->http_req->data, parser_handle->parse_offset, parser_handle);
        if (read_result <= 0) {
            break; // Stop if no more data is read
        }
    }

    return -1; // Key not found
}

int parse_req_payload(void *http_parser_handle, char **payload, int *length) {
    http_parser_handle_t *parser_handle = (http_parser_handle_t *)http_parser_handle;

    while (1) {
        // Start searching from the zero offset
        char *payload_start = strstr(parser_handle->http_req->data, "\r\n\r\n");
        if (payload_start == NULL || payload_start >= parser_handle->http_req->data + parser_handle->parse_offset) {
            // If not found, invoke the read callback
            ssize_t read_result = parser_handle->read_callback(parser_handle->http_req->data, parser_handle->parse_offset, parser_handle);
            if (read_result <= 0) {
                return -1; // Stop if no more data is read
            }
            continue; // Retry after reading more data
        }
        payload_start += 4; // Move past the "\r\n\r\n"
        char *payload_end = strchr(payload_start, '\r');
        if (payload_end == NULL || payload_end > parser_handle->http_req->data + parser_handle->parse_offset) {
            payload_end = payload_start + strlen(payload_start); // If no newline, use the end of the string
        }
        size_t payload_length = payload_end - payload_start;

        // Check the length of the payload from the headers
        char *content_length_str;
        int content_length;
        if (parse_get_hdr_value(http_parser_handle, "Content-Length", &content_length_str, &content_length) == 0) {
            int header_payload_length = atoi(content_length_str);
            if (payload_length < header_payload_length) {
                // If actual length of payload is less, invoke the read callback
                ssize_t read_result = parser_handle->read_callback(parser_handle->http_req->data, parser_handle->parse_offset, parser_handle);
                if (read_result <= 0) {
                    return -1; // Stop if no more data is read
                }
                continue; // Retry after reading more data
            }
        }

        *payload = strndup(payload_start, payload_length);
        if (*payload == NULL) {
            return -1; // Memory allocation failed
        }

        // Update the length to return
        *length = payload_length;

        // Update the parse offset to the end of the payload
        parser_handle->parse_offset = payload_end - parser_handle->http_req->data;

        return 0;
    }
}