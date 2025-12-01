/*
    @author yuan
    @brief  a config file parser, written in C.

    this config file's format is very simple, just like: key = value, for example, if you have a file called config.txt,
    then the file content:

        thread_pool_size = 256
        port_start = 0
        port_end = 65535
        timeout_millisec = 2000

    spaces would be ignored, so the format can be beautified as much as possible.
*/
#ifndef _LIB_CONFIG_PARSER_H_
#define _LIB_CONFIG_PARSER_H_

#include <stddef.h>

struct ConfigData {
    char* key;
    char* value;
    int keyLen;
    int valueLen;
    struct ConfigData* next;
};

struct ConfigParser {
    struct ConfigData** bucket;
    size_t len;
    size_t bucketSize;
};

#define CONFIG_PARSER_ERROR_OK                0
#define CONFIG_PARSER_ERROR_FILE_OPEN_FAILED  1
#define CONFIG_PARSER_ERROR_OUT_OF_MEMORY     2

/* 
    init.

    if success, return CONFIG_PARSER_ERROR_OK,
    else return a error code.
*/
int config_parser_init(struct ConfigParser* parser, size_t bucketSize);

/* destroy. */
void config_parser_destroy(struct ConfigParser* parser);

/* format error code. */
const char* config_parser_strerr(int code);

/* 
    parse the given file. 
    
    if success, return CONFIG_PARSER_ERROR_OK,
    else return a error code.
*/
int config_parser_load_file(struct ConfigParser* parser, const char* filePath, int lineLengthLimit);

/* get data from the given key. if not exists, return NULL. */
struct ConfigData* config_parser_get(const struct ConfigParser* parser, const char* key);

/* print all configs, for debug usage. */
void config_parser_print_all(const struct ConfigParser* parser);

#endif
