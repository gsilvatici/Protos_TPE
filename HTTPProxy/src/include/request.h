#include <stdint.h>
#include <stdbool.h>

#include <netinet/in.h>

#include "buffer.h"
#include <string.h>

enum http_method {
    get_method  = 0x01,
    post_method = 0x02,
    head_method = 0x03,
};

enum url_state {
  url_init_state,
  url_schema_state,
  url_slash_state,
  url_slash_slash_state,
  url_auth_f_state,
  url_auth_userinfo_state,
  url_auth_host_state,
  url_auth_port_state,
  url_ipv6_state,
  url_path_state,
  url_query_state,
  url_done_state,
  url_invalid_state,
};

enum header_state {
  header_init,
  header_name,
  header_value,
  header_value_start,
  header_done_cr,
  header_done,
  header_content_length_check,
  header_host_check,
 };

enum http_state {
  method_state,
  check_method_state,
  absolute_url_state,
  version_state,
  start_header_state,
  headers_state,
  body_start_state,
  body_state,
  done_state,
  error_unsupported_method_state,
  error_too_long_url_state,
  error_too_long_header_state,
  error_invalid_url_state,
  error_unsupported_version_state,
  error_no_end_state,
  error_bad_request_state,
  error_unsupported_encoding_state,
  error_code_notsupported_state,
  error_reason_too_long_state,
  error_bad_response_state,
  sp_state,
  status_code_state,
  status_rn_state,
};

struct http_parser {
  struct http_request *request;
  enum http_state state;
  enum url_state url_state;
  enum header_state h_state;
  uint16_t i_host;
  uint16_t i_header;
  bool  host_defined;
  uint32_t content_length;
  bool body_found;
  bool is_proxy_connection;
  bool method_supported;
   /** cuantos bytes tenemos que leer*/
   uint16_t n;
   /** cuantos bytes ya leimos */
   uint16_t i;
};

struct http_request {
  enum http_method method;
  char http_version;
  char bckp_headers[MAX_HEADERS_LEN_ARRAY];
  char absolute_url[MAX_URL_LEN];
  char fqdn[MAX_FQDN_LEN];
  char header_host[MAX_FQDN_LEN];
  char * headers;
  in_port_t dest_port;
  uint32_t header_content_len;
};


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