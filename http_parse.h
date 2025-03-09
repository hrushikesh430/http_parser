#include<stdio.h>

struct http_req {
    char *data;            /* http request data */
    size_t data_len;       /* Length of the data buffer */
};

struct parser_data {
    char *method;          /* HTTP method */
    int method_len;        /* Length of the method */
    char *uri;             /* URI of the request */
    int uri_len;           /* Length of the URI */
    char *query;           /* Query of the request */
    int query_len;         /* Length of the query */
    char *payload;         /* Payload of the request */
    int payload_len;       /* Length of the payload */
};

typedef struct http_req http_req_t;
typedef struct parser_data parser_data_t;

/**
 * @brief This api will init the parser_cfg from
 *        provided parser_usr_cfg.
 *        NOTE - This api should be called before any
 *        parse_* api.
 *
 * @param http_req pointer to the http_req
 * 
 * @return void* for http_parser_handle, or NULL on failure
 */
void* parser_init(http_req_t *http_req);

/**
 * @brief Parse the new request coming from the server
 * 
 * @param http_parser_handle pointer to the http_parser_handle
 * 
 * @return 1 on success, 0 on failure
 */
int parse_new_req(void *http_parser_handle);

/**
 * @brief Parse the URI from the request 
 * 
 * @param http_parser_handle pointer to the http_parser_handle
 * @param buf pointer to the buffer, where the URI is stored
 * @param buf_len Length of the URI
 * 
 * @return Offset of the URI in the buffer on success, -1 on failure
 */
int parse_get_uri(void *http_parser_handle, char **buf, int *buf_len);


/**
 * @brief Fills value buffer the value of given key
 * 
 * @param http_parser_handle pointer to the http_parser_handle
 * @param key pointer to key buffer, where the key string is stored
 * @param length Length of the value
 * 
 * @return Offset of the value in the buffer on success, -1 on failure
 */
int parse_get_hdr_value(void *http_parser_handle, char *key, char **value, int *length);

/**
 * @brief Fills the query buffer the URI query
 * 
 * @param http_parser_handle pointer to the http_parser_handle
 * @param length Length of the query
 * 
 * @return Offset of the query in the buffer on success, -1 on failure
 */
int parse_uri_query_param(void *http_parser_handle, char *key, char **value, int *length);

/**
 * @brief Fills the payload buffer, with the payload
 *        with request payload.
 *
 * @param http_parser_handle pointer to the http_parser_handle
 * @param length Length of the payload
 * 
 * @return Offset of the payload in the buffer on success, -1 on failure
 */
int parse_req_payload(void *http_parser_handle, char **payload, int *length);