/**
 * request.c -- parser del request de HTTP
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "request.h"


static void
remaining_set(struct http_parser* parser, const int n) {
    parser->i = 0;
    parser->n = n;
}

static int
remaining_is_done(struct http_parser *parser) {
    return parser->i >= parser->n;
}

//////////////////////////////////////////////////////////////////////////////

extern enum http_state 
http_parser_feed(struct http_parser *parser, uint8_t b) {
    enum http_state next;

    switch(parser->state) {
        case method_state:
            next = get_http_method(b, parser);
            break;
        case check_method_state:
            next = parse_method(b, parser);
            break;
        case absolute_url_state:
            next = url_check(b, parser);
            break;
        case version_state:
            next = version(b, parser);
            break;
        case http_done_cr:
        case http_done_cr_cr:
            next = error_no_end_state;
            if(b == LF){
                next = start_header_state;
            }
            if(b == CR){
                next = http_done_cr_cr;
            }
            break;
        case start_header_state:
            parser->h_state = header_init;
            if(b == CR){
                next = body_start_state;
                break;
            }
        case headers_state:
            next = header_check(b, parser);
            break;
        case body_start_state:
            next = error_bad_request_state;
            if(b == LF){
                next = body_state;
            }
            break;
        case body_state:
            parser->body_found = true;
            break;
        case done_state:
        case error_unsupported_method_state:
        case error_too_long_url_state:
        case error_invalid_url_state:
        case error_unsupported_version_state:
        case error_no_end_state:
        case error_too_long_header_state:
        case error_bad_request_state:
            break;
        default:
            abort();
    }

    return parser->state = next;
}


extern bool 
http_is_done(const enum http_state state, bool *errored) {
    bool ret;
    switch (state) {
        case error_unsupported_method_state:
        case error_too_long_url_state:
        case error_invalid_url_state:
        case error_unsupported_version_state:
        case error_bad_request_state:
        case error_no_end_state:
            if (0 != errored) {
                *errored = true;
            }
        case body_state:
        case done_state:
            ret = true;
            break;
        default:
            ret = false;
            break;
    }
   return ret;
}

extern const char *
http_error(const struct http_parser *parser) {
    char *err;

    switch (parser->state) {
        case error_unsupported_method_state:
            err = "method not supported";
            break;
        case error_too_long_url_state:
            err = "url too long";
            break;
        case error_invalid_url_state:
            err = "invalid url";
            break;
        case error_no_end_state:
            err = "no end";
        case error_unsupported_version_state:
            err = "unsupported version";
            break;
        default:
            err = "";
            break;
    }
    return err;
}

extern void http_parser_close(struct http_parser *parser) {
    /* no hay nada que liberar */
}

extern enum http_state
http_consume(buffer *b, struct http_parser *parser, bool *errored) {

    enum http_state st = parser->state;
    while(buffer_can_read(b)) { 
        const uint8_t c = buffer_read(b);
        st = http_parser_feed(parser, c);
        if (http_is_done(st, errored)){
            break;
        }
    }
    return st;
}