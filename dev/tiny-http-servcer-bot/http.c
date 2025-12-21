#include "http.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>

/* メモリプール上で文字列の指定長分を複製 */
static char *pool_strndup(MemPool *pool, const char *src, size_t len) {
    /* 必要な領域を確保（ヌル終端1文字枠追加） */
    char *dst = (char *)mm_pool_alloc(pool, len + 1U);
    if (!dst) {
        return NULL;
    }
    /* 空間を含めて指定長をコピー */
    memcpy(dst, src, len);
    dst[len] = '\0';
    return dst;
}

/* 文字列の先頭と末尾の空白を削除 */
static void trim(char **start, char **end) {
    /* ポインタ指す位置から先頭空白をスキップ */
    while (*start < *end && isspace((unsigned char)**start)) {
        (*start)++;
    }
    /* 末尾ざぎき空白を削除 */
    while (*end > *start && isspace((unsigned char)*((*end) - 1))) {
        (*end)--;
    }
}

/* HTTPヘッダー値を「,」「;」で分割してトークン化（元文字列は不変で保持） */
static int parse_header_items(const char *value, HttpHeaderValue *out, MemPool *pool) {
    /* トークン配列の初期容量を一定数設定 */
    size_t cap = 4;
    out->items = (char **)mm_pool_alloc(pool, cap * sizeof(char *));
    if (!out->items) {
        return -1;
    }
    out->item_count = 0;
    /* 元のヘッダー値全体を一端不変に保持 */
    out->raw = (char *)value;

    /* 区切りで切り出し、前後空白を除去したトークンをコピー保存 */
    const char *p = value;
    while (*p) {
        const char *token_start = p;
        while (*p && *p != ',' && *p != ';') {
            p++;
        }
        const char *token_end = p;
        char *ts = (char *)token_start;
        char *te = (char *)token_end;
        trim(&ts, &te);
        if (te > ts) {
            if (out->item_count == cap) {
                size_t new_cap = cap * 2U;
                char **new_items = (char **)mm_pool_alloc(pool, new_cap * sizeof(char *));
                if (!new_items) {
                    return -1;
                }
                memcpy(new_items, out->items, cap * sizeof(char *));
                out->items = new_items;
                cap = new_cap;
            }
            size_t tok_len = (size_t)(te - ts);
            char *copy = (char *)mm_pool_alloc(pool, tok_len + 1U);
            if (!copy) {
                return -1;
            }
            memcpy(copy, ts, tok_len);
            copy[tok_len] = '\0';
            out->items[out->item_count++] = copy;
        }
        if (*p == '\0') {
            break;
        }
        p++;
    }
    return 0;
}

/* ヘッダー配列に要素を追加する共通処理（リクエスト/レスポンス双方で使用） */
static int add_header_span(HttpHeader **headers, size_t *count, size_t *capacity,
                           const char *name_start, size_t name_len,
                           const char *value_start, size_t value_len,
                           MemPool *pool) {
    if (!headers || !count || !capacity || !pool) {
        return -1;
    }
    /* 初回確保 */
    if (!*headers) {
        *capacity = 8U;
        *headers = (HttpHeader *)mm_pool_alloc(pool, (*capacity) * sizeof(HttpHeader));
        if (!*headers) {
            return -1;
        }
        *count = 0;
    }
    /* 必要に応じて容量拡張 */
    if (*count == *capacity) {
        size_t new_cap = (*capacity) * 2U;
        HttpHeader *new_headers = (HttpHeader *)mm_pool_alloc(pool, new_cap * sizeof(HttpHeader));
        if (!new_headers) {
            return -1;
        }
        memcpy(new_headers, *headers, (*capacity) * sizeof(HttpHeader));
        *headers = new_headers;
        *capacity = new_cap;
    }

    /* ヘッダー名・値をプールに複製 */
    HttpHeader *hdr = &(*headers)[*count];
    char *name = pool_strndup(pool, name_start, name_len);
    char *value = pool_strndup(pool, value_start, value_len);
    if (!name || !value) {
        return -1;
    }
    hdr->name = name;
    if (parse_header_items(value, &hdr->value, pool) != 0) {
        return -1;
    }
    (*count)++;
    return 0;
}

/* HTTPリクエスト一行目をパース（メソッド, パス, HTTPバージョン） */
static int parse_request_line(const char *line_start, const char *line_end, HttpRequest *req, MemPool *pool) {
    /* 第1空白でメソッド終端を特定 */
    const char *sp1 = memchr(line_start, ' ', (size_t)(line_end - line_start));
    if (!sp1) {
        return -1;
    }
    /* 第2空白でパス終端を特定 */
    const char *sp2 = memchr(sp1 + 1, ' ', (size_t)(line_end - sp1 - 1));
    if (!sp2) {
        return -1;
    }
    /* 各要素をメモリプール上に複製 */
    req->method = pool_strndup(pool, line_start, (size_t)(sp1 - line_start));
    req->path = pool_strndup(pool, sp1 + 1, (size_t)(sp2 - sp1 - 1));
    req->version = pool_strndup(pool, sp2 + 1, (size_t)(line_end - sp2 - 1));
    if (!req->method || !req->path || !req->version) {
        return -1;
    }
    return 0;
}

/* HttpRequest 構造体をゼロ初期化する */
void http_request_init(HttpRequest *req) {
    if (req) {
        memset(req, 0, sizeof(*req));
    }
}

/* リクエストラインを設定する（ビルド用） */
int http_set_request_line(HttpRequest *req, const char *method, const char *path, const char *version, MemPool *pool) {
    if (!req || !method || !path || !version || !pool) {
        return -1;
    }
    req->method = mm_pool_strdup(pool, method);
    req->path = mm_pool_strdup(pool, path);
    req->version = mm_pool_strdup(pool, version);
    if (!req->method || !req->path || !req->version) {
        return -1;
    }
    return 0;
}

/* HTTPリクエストにヘッダーを追加（外部API） */
int http_add_header(HttpRequest *req, const char *name, const char *value, MemPool *pool) {
    if (!req || !name || !value || !pool) {
        return -1;
    }
    size_t name_len = strlen(name);
    size_t value_len = strlen(value);
    return add_header_span(&req->headers, &req->header_count, &req->header_capacity,
                           name, name_len, value, value_len, pool);
}

/* HTTPリクエスト全体をパース */
int http_parse_request(const char *data, size_t len, HttpRequest *out, MemPool *pool) {
    /* 入力確認 */
    if (!data || !out || !pool) {
        return -1;
    }
    /* 出力構造体を初期化 */
    memset(out, 0, sizeof(*out));

    const char *p = data;
    const char *end = data + len;

    /* 第1行（リクエスト一行目）を検索してパース */
    const char *line_end = NULL;
    for (const char *q = p; q + 1 < end; ++q) {
        if (q[0] == '\r' && q[1] == '\n') {
            line_end = q;
            break;
        }
    }
    if (!line_end) {
        return -1;
    }
    if (parse_request_line(p, line_end, out, pool) != 0) {
        return -1;
    }
    /* 次行から開始 */
    p = line_end + 2;

    /* ヘッダーを走査し、空行で抜ける迄パース */
    while (p < end) {
        /* 次の空行（ボディの開始）を検出 */
        if ((end - p) >= 2 && p[0] == '\r' && p[1] == '\n') {
            p += 2;
            break;
        }
        const char *line_start = p;
        line_end = NULL;
        for (const char *q = p; q + 1 < end; ++q) {
            if (q[0] == '\r' && q[1] == '\n') {
                line_end = q;
                break;
            }
        }
        if (!line_end) {
            return -1;
        }
        p = line_end + 2;

        /* コロン（：）を基準にヘッダー名と値を分割 */
        const char *colon = memchr(line_start, ':', (size_t)(line_end - line_start));
        if (!colon) {
            return -1;
        }
        char *name_start = (char *)line_start;
        char *name_end = (char *)colon;
        trim(&name_start, &name_end);

        char *val_start = (char *)(colon + 1);
        char *val_end = (char *)line_end;
        trim(&val_start, &val_end);

        if (name_end <= name_start || val_end < val_start) {
            return -1;
        }
        if (add_header_span(&out->headers, &out->header_count, &out->header_capacity,
                            name_start, (size_t)(name_end - name_start),
                            val_start, (size_t)(val_end - val_start),
                            pool) != 0) {
            return -1;
        }
    }

    /* ボディを設定 */
    if (p < end) {
        out->body = (char *)p;
        out->body_length = (size_t)(end - p);
    }
    return 0;
}

/* ヘッダー名で検索し、raw 値を取得（大文字小文字無視） */
const char *http_get_header(const HttpRequest *req, const char *name) {
    if (!req || !name) {
        return NULL;
    }
    for (size_t i = 0; i < req->header_count; ++i) {
        if (strcasecmp(req->headers[i].name, name) == 0) {
            return req->headers[i].value.raw;
        }
    }
    return NULL;
}

/* HttpResponse 構造体をゼロ初期化する */
void http_response_init(HttpResponse *res) {
    if (res) {
        memset(res, 0, sizeof(*res));
    }
}

/* ステータスコードと Reason-Phrase を設定 */
int http_response_set_status(HttpResponse *res, int status_code, const char *reason, MemPool *pool) {
    if (!res || !reason || !pool) {
        return -1;
    }
    res->status_code = status_code;
    res->reason = mm_pool_strdup(pool, reason);
    if (!res->reason) {
        return -1;
    }
    return 0;
}

/* レスポンスボディを設定（ポインタを保持するだけ。必要なら呼び出し側でプールに載せる） */
int http_response_set_body(HttpResponse *res, const char *body, size_t body_length) {
    if (!res) {
        return -1;
    }
    res->body = body;
    res->body_length = body_length;
    return 0;
}

/* レスポンスヘッダーを追加 */
int http_response_add_header(HttpResponse *res, const char *name, const char *value, MemPool *pool) {
    if (!res || !name || !value || !pool) {
        return -1;
    }
    return add_header_span(&res->headers, &res->header_count, &res->header_capacity,
                           name, strlen(name), value, strlen(value), pool);
}

/* レスポンスをシリアライズして raw/raw_length に格納（プール上のバッファ） */
int http_response_build(HttpResponse *res, MemPool *pool) {
    if (!res || !pool || !res->reason) {
        return -1;
    }
    /* ステータスコード文字列表現を作成 */
    char code_buf[16];
    int code_len = snprintf(code_buf, sizeof(code_buf), "%d", res->status_code);
    if (code_len <= 0 || (size_t)code_len >= sizeof(code_buf)) {
        return -1;
    }

    /* 全体長を事前計算 */
    const char *version = "HTTP/1.1";
    size_t total = 0;
    total += strlen(version) + 1 /* space */ + (size_t)code_len + 1 /* space */ + strlen(res->reason) + 2 /* CRLF */;
    for (size_t i = 0; i < res->header_count; ++i) {
        HttpHeader *h = &res->headers[i];
        total += strlen(h->name) + 2 /* : */ + strlen(h->value.raw) + 2 /* CRLF */;
    }
    total += 2; /* 空行 CRLF */
    total += res->body_length;

    /* バッファをプールから確保 */
    char *buf = (char *)mm_pool_alloc(pool, total + 1U);
    if (!buf) {
        return -1;
    }

    /* 書き込み */
    char *w = buf;
    size_t version_len = strlen(version);
    memcpy(w, version, version_len); w += version_len;
    *w++ = ' ';
    memcpy(w, code_buf, (size_t)code_len); w += code_len;
    *w++ = ' ';
    size_t reason_len = strlen(res->reason);
    memcpy(w, res->reason, reason_len); w += reason_len;
    *w++ = '\r'; *w++ = '\n';

    for (size_t i = 0; i < res->header_count; ++i) {
        HttpHeader *h = &res->headers[i];
        size_t name_len = strlen(h->name);
        size_t raw_len = strlen(h->value.raw);
        memcpy(w, h->name, name_len); w += name_len;
        *w++ = ':'; *w++ = ' ';
        memcpy(w, h->value.raw, raw_len); w += raw_len;
        *w++ = '\r'; *w++ = '\n';
    }
    /* 空行 */
    *w++ = '\r'; *w++ = '\n';

    /* ボディ */
    if (res->body && res->body_length > 0) {
        memcpy(w, res->body, res->body_length);
        w += res->body_length;
    }

    *w = '\0';
    res->raw = buf;
    res->raw_length = (size_t)(w - buf);
    return 0;
}
