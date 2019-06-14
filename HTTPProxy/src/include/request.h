#include <stdint.h>
#include <stdbool.h>

#include <netinet/in.h>

#include "buffer.h"
#include <string.h>



/** inicializa el parser */
void http_parser_init (struct http_parser *p);

/** entrega un byte al parser. retorna true si se llego al final  */
enum http_state http_parser_feed (struct http_parser *p, uint8_t b);

/**
 * por cada elemento del buffer llama a `hello_parser_feed' hasta que
 * el parseo se encuentra completo o se requieren mas bytes.
 *
 * @param errored parametro de salida. si es diferente de NULL se deja dicho
 *   si el parsing se debió a una condición de error
 */
enum http_state
http_consume(buffer *b, struct http_parser *p, bool *errored);

/**
 * Permite distinguir a quien usa hello_parser_feed si debe seguir
 * enviando caracters o no. 
 *
 * En caso de haber terminado permite tambien saber si se debe a un error
 */
bool 
http_is_done(const enum http_state state, bool *errored);

/**
 * En caso de que se haya llegado a un estado de error, permite obtener una
 * representación textual que describe el problema
 */
extern const char *
http_error(const struct http_parser *p);


/** libera recursos internos del parser */
void http_parser_close(struct http_parser *p);

/**
 * serializa en buff la una respuesta al http.
 *
 * Retorna la cantidad de bytes ocupados del buffer o -1 si no había
 * espacio suficiente.
 */
int
http_marshall(buffer *b, struct http_request * req, buffer *b2);