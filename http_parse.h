#include<stdio.h>


struct parser_cfg {
    char *read_buf;            /* Buffer the server request will be read */
    int read_buf_len;          /* Length of the read buffer */
    char *parser_buf;          /* Buffer where the parsed data will be stored */
    int parser_buf_len;        /* Length of the parsed buffer */
};

struct parser_usr_cfg {
    int read_buf_len_max;          /* Maximum length of the read buffer */
    int parser_buf_len_max;        /* Maximum length of the parser buffer */
};

struct http_req {
    char *data;         /* http request data */
    int content_len;       /* Length of the http request content */
};

typedef struct parser_cfg parser_cfg_t;
typedef struct parser_usr_cfg parser_usr_cfg_t;
typedef struct http_req http_req_t;

/**
 * @brief This api will init the parser_cfg from
 *        provided parser_usr_cfg.
 *        NOTE - This api should be called before any
 *        parse_* api.
 *
 * @param parser_usr_cfg pointer to the parser_usr_cfg
 * 
 * @return void
 */
void parser_init(parser_usr_cfg_t *parser_usr_cfg);


/**
 * @brief Parse the new request coming from the server
 * 
 * @param http_req http_req_t struct of the request and want to
 *                 parse
 * 
 * @return void
 */
void parse_new_req(http_req_t http_req);

/**
 * @brief Parse the URI from the request 
 * 
 * @param uri Uri buffer to store URI from the request 
 * 
 * @return void
 */
void parse_get_uri(char *uri);


/**
 * @brief Fills value buffer the value of given key
 * 
 * @param key pointer to key buffer, where the key string is stored
 * @param value pointer to value buffer, where the value will be stored
 * 
 * @return void
 */
void parse_get_hdr_value(char *key, char *value);

/**
 * @brief Fills the query buffer the URI query
 * 
 * @param query Fills the query buffer with if query if there
 *              or points to NULL
 * 
 * @return void
 */
void parse_uri_query_param(char *query);

/**
 * @brief Fills the payload buffer, with the payload
 *        with request payload.
 *
 * @param payload Payload buffer
 * 
 * @return void
 */
void parse_req_payload(char *payload);