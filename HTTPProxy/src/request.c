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

