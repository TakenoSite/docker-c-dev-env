#include "http.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

static void test_parse_request(void) {
    const char *raw =
        "POST /submit HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Accept: text/html, application/json; q=0.9\r\n"
        "X-Custom: token1; token2,token3\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "hello";

    MemPool *pool = mm_pool_create(2048, 1);
    assert(pool);
    HttpRequest req;
    int rc = http_parse_request(raw, strlen(raw), &req, pool);
    assert(rc == 0);

    assert(strcmp(req.method, "POST") == 0);
    assert(strcmp(req.path, "/submit") == 0);
    assert(strcmp(req.version, "HTTP/1.1") == 0);
    assert(req.header_count == 5);

    const char *ct = http_get_header(&req, "Content-Type");
    assert(ct && strcmp(ct, "text/html; charset=UTF-8") == 0);

    const char *accept_raw = http_get_header(&req, "accept");
    assert(accept_raw && strstr(accept_raw, "application/json"));

    /* Tokenization for comma/semicolon separated values */
    HttpHeader *accept_hdr = NULL;
    for (size_t i = 0; i < req.header_count; ++i) {
        if (strcasecmp(req.headers[i].name, "Accept") == 0) {
            accept_hdr = &req.headers[i];
            break;
        }
    }
    assert(accept_hdr);
    assert(accept_hdr->value.item_count == 3);
    assert(strcmp(accept_hdr->value.items[0], "text/html") == 0);
    assert(strcmp(accept_hdr->value.items[1], "application/json") == 0);
    assert(strcmp(accept_hdr->value.items[2], "q=0.9") == 0);

    HttpHeader *xc = NULL;
    for (size_t i = 0; i < req.header_count; ++i) {
        if (strcasecmp(req.headers[i].name, "X-Custom") == 0) {
            xc = &req.headers[i];
            break;
        }
    }
    assert(xc);
    assert(xc->value.item_count == 3);
    assert(strcmp(xc->value.items[0], "token1") == 0);
    assert(strcmp(xc->value.items[1], "token2") == 0);
    assert(strcmp(xc->value.items[2], "token3") == 0);

    assert(req.body && req.body_length == 5);
    assert(strncmp(req.body, "hello", 5) == 0);

    mm_pool_destroy(pool);
}

static void test_build_response(void) {
    MemPool *pool = mm_pool_create(2048, 1);
    assert(pool);

    HttpResponse res;
    http_response_init(&res);
    assert(http_response_set_status(&res, 201, "Created", pool) == 0);
    assert(http_response_add_header(&res, "Content-Type", "application/json", pool) == 0);

    const char body[] = "{\"ok\":true}";
    assert(http_response_set_body(&res, body, strlen(body)) == 0);
    assert(http_response_add_header(&res, "Content-Length", "11", pool) == 0);
    assert(http_response_build(&res, pool) == 0);

    const char expected[] =
        "HTTP/1.1 201 Created\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 11\r\n"
        "\r\n"
        "{\"ok\":true}";

    assert(res.raw_length == strlen(expected));
    assert(strcmp(res.raw, expected) == 0);

    mm_pool_destroy(pool);
}

int main(void) {
    test_parse_request();
    test_build_response();
    printf("test_http (parse/response) passed\n");
    return 0;
}
