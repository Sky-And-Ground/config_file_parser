#include "lib_config_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static unsigned int config_parser_hash(const struct ConfigParser* parser, const char* key) {
    unsigned int hashval = 0;

    while (*key) {
        hashval = 131 * hashval + (unsigned int)(*key - '0');
        ++key;
    }

    return hashval % parser->bucketSize;
}

/*
    read a line from the file(not binary file).
    return the read length, and always ends with a '\0'.

    if the line is too long, then this function would just read limit - 1 characters, and ignore this line's remaining parts.
*/
static int read_line(char* buf, int limit, FILE* f) {
    int len = 0;

    while (1) {
        char c = fgetc(f);

        if (c == '\n' || c == EOF) {
            break;
        }

        buf[len] = c;

        if (len + 1 < limit) {
            ++len;
        }
    }

    buf[len] = '\0';
    return len;
}

static void config_parser_add(struct ConfigParser* parser, struct ConfigData* data) {
    struct ConfigData* node = config_parser_get(parser, data->key);

    if (node) {  /* no update. */
        return;
    }
    else {
        unsigned int hashval = config_parser_hash(parser, data->key);

        if (parser->bucket[hashval] == NULL) {
            parser->bucket[hashval] = data;
        }
        else {
            data->next = parser->bucket[hashval];
            parser->bucket[hashval] = data;
        }
        
        parser->len += 1;
    }
}

static struct ConfigData* parse_line(const char* str, int len) {
    struct ConfigData* data = NULL;
    const char* keyStart = NULL;
    const char* valueStart = NULL;
    int keyLen = 0;
    int valueLen = 0;
    int i = 0;
    int tmp;

    /* skip spaces. */
    while (i < len && isspace(str[i])) {
        ++i;
    }

    if (i >= len) {
        return NULL;
    }

    /* get key position. */
    keyStart = str + i;

    while (i < len && !isspace(str[i])) {
        ++i;
        ++keyLen;
    }

    if (i >= len) {
        return NULL;
    }

    /* get '=' position. */
    while (i < len && str[i] != '=') {
        ++i;
    }

    if (i >= len) {
        return NULL;
    }

    /* skip spaces. */
    ++i;
    while (i < len && isspace(str[i])) {
        ++i;
    }

    if (i >= len) {
        return NULL;
    }

    /* get value position. */
    valueStart = str + i;
    valueLen = len - i;

    tmp = len - 1;
    while (tmp > i && isspace(str[tmp])) {
        --tmp;
        --valueLen;
    }

    /* build data. */
    data = (struct ConfigData*)malloc(sizeof(struct ConfigData));

    if (data == NULL) {
        return NULL;
    }

    data->key = (char*)malloc(keyLen + valueLen + 2);
    if (data->key == NULL) {
        free(data);
        return NULL;
    }

    memcpy(data->key, keyStart, keyLen);
    data->key[keyLen] = '\0';
    data->keyLen = keyLen;

    data->value = data->key + keyLen + 1;
    memcpy(data->value, valueStart, valueLen);
    data->value[valueLen] = '\0';
    data->valueLen = valueLen;

    data->next = NULL;
    return data;
}

int config_parser_init(struct ConfigParser* parser, size_t bucketSize) {
    int i;

    parser->bucket = (struct ConfigData**)malloc(bucketSize * sizeof(struct ConfigData*));
    if (parser->bucket == NULL) {
        return CONFIG_PARSER_ERROR_OUT_OF_MEMORY;
    }

    for (i = 0; i < bucketSize; ++i) {
        parser->bucket[i] = NULL;
    }

    parser->bucketSize = bucketSize;
    parser->len = 0;
    return CONFIG_PARSER_ERROR_OK;
}

void config_parser_destroy(struct ConfigParser* parser) {
    int i;
    for (i = 0; i < parser->bucketSize; ++i) {
        struct ConfigData* cursor = parser->bucket[i];
        struct ConfigData* tmp;

        while (cursor) {
            tmp = cursor->next;

            /* key and value are allocated in the same block, so just free the key. */
            free(cursor->key);
            free(cursor);

            cursor = tmp;
        }
    }
}

const char* config_parser_strerr(int code) {
    switch(code) {
        case CONFIG_PARSER_ERROR_OK:
            return "ok";
        case CONFIG_PARSER_ERROR_FILE_OPEN_FAILED:
            return "file cannot open";
        case CONFIG_PARSER_ERROR_OUT_OF_MEMORY:
            return "out of memory";
        default:
            return "unknown config parser error";
    }
}

int config_parser_load_file(struct ConfigParser* parser, const char* filePath, int lineLengthLimit) {
    FILE* f = fopen(filePath, "r");
    
    if (f == NULL) {
        return CONFIG_PARSER_ERROR_FILE_OPEN_FAILED;
    }
    else {
        char* buf;
        int len = 0;
        struct ConfigData* data;

        buf = (char*)malloc(lineLengthLimit * sizeof(char));
        if (buf == NULL) {
            fclose(f);
            return CONFIG_PARSER_ERROR_OUT_OF_MEMORY;
        }

        while (!feof(f)) {
            len = read_line(buf, lineLengthLimit, f);

            if (len == 0) {
                continue;
            }

            data = parse_line(buf, len);
            
            if (data != NULL) {
                config_parser_add(parser, data);
            }
        }

        free(buf);
    }

    fclose(f);
    return CONFIG_PARSER_ERROR_OK;
}

struct ConfigData* config_parser_get(const struct ConfigParser* parser, const char* key) {
    unsigned int hashval = config_parser_hash(parser, key);
    struct ConfigData* cursor = parser->bucket[hashval];
    
    while (cursor) {
        if (strcmp(key, cursor->key) == 0) {
            return cursor;
        }

        cursor = cursor->next;
    }
    
    return NULL;
}

void config_parser_print_all(const struct ConfigParser* parser) {
    int i;
    struct ConfigData* cursor;

    for (i = 0; i < parser->bucketSize; ++i) {
        cursor = parser->bucket[i];

        while (cursor) {
            printf("%s: %s\n", cursor->key, cursor->value);
            cursor = cursor->next;
        }
    }
}
