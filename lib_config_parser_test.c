#include "lib_config_parser.h"

int main(void) {
    struct ConfigParser parser;

    config_parser_init(&parser, 101);
    config_parser_load_file(&parser, "port_scanner.txt", 256);
    config_parser_print_all(&parser);
    config_parser_destroy(&parser);
    return 0;
}
