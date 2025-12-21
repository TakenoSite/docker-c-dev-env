#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>
#include "mm_pool.h"

/*
 * HTTPヘッダーの値を表す構造体。
 * raw: 元の文字列（カンマ・セミコロン区切りを含む完全な値）
 * items: 区切り文字（',' または ';'）で分割したトークン配列
 * item_count: トークン数
 */
typedef struct HttpHeaderValue {
    char *raw;        /* 元のヘッダー値（不変） */
    char **items;     /* 分割後のトークン配列 */
    size_t item_count;/* トークン数 */
} HttpHeaderValue;

/*
 * 1つのHTTPヘッダー行を表す構造体。
 * name: ヘッダー名（例: "Content-Type"）
 * value: ヘッダー値（生文字列とトークン化済みの配列）
 */
typedef struct HttpHeader {
    char *name;            /* ヘッダー名 */
    HttpHeaderValue value; /* ヘッダー値 */
} HttpHeader;

/*
 * HTTPリクエスト全体を表す構造体。
 * method: リクエストメソッド（GET, POST など）
 * path: リクエストターゲット（パス）
 * version: HTTPバージョン文字列（例: "HTTP/1.1"）
 * headers: ヘッダー配列（メモリプール上に確保）
 * header_count: 現在保持しているヘッダー数
 * header_capacity: headers配列の確保済み容量（プール上で段階的に拡張）
 * body: メッセージボディへのポインタ（入力バッファ内参照）
 * body_length: ボディ長（バイト数）
 */
typedef struct HttpRequest {
    char *method;           /* メソッド */
    char *path;             /* パス */
    char *version;          /* HTTPバージョン */
    HttpHeader *headers;    /* ヘッダー配列 */
    size_t header_count;    /* 保持ヘッダー数 */
    size_t header_capacity; /* 配列容量 */
    char *body;             /* ボディ先頭 */
    size_t body_length;     /* ボディ長 */
} HttpRequest;

/*
 * HTTPレスポンス全体を表す構造体。
 * status_code: ステータスコード (例: 200)
 * reason: Reason-Phrase (例: "OK")
 * headers: ヘッダー配列（メモリプール上に確保）
 * body/body_length: ボディ先頭ポインタと長さ
 * raw/raw_length: 生成されたレスポンス文字列とその長さ
 */
typedef struct HttpResponse {
    int status_code;        /* ステータスコード */
    char *reason;           /* Reason-Phrase */
    HttpHeader *headers;    /* ヘッダー配列 */
    size_t header_count;    /* 保持ヘッダー数 */
    size_t header_capacity; /* 配列容量 */
    const char *body;       /* ボディ先頭（外部またはプール上） */
    size_t body_length;     /* ボディ長 */
    char *raw;              /* シリアライズ済みレスポンス */
    size_t raw_length;      /* シリアライズ長 */
} HttpResponse;

/*
 * HTTPリクエスト文字列をパースして HttpRequest に格納する。
 * data/len: 入力バッファとその長さ
 * out: 結果の構造体（フィールドはプール上に割り当て）
 * pool: メモリプール（断片化・解放漏れ防止のため使用）
 * 戻り値: 0 成功 / -1 失敗
 */
int http_parse_request(const char *data, size_t len, HttpRequest *out, MemPool *pool);

/* HttpRequest の簡易ビルダーAPI */
void http_request_init(HttpRequest *req);
int http_set_request_line(HttpRequest *req, const char *method, const char *path, const char *version, MemPool *pool);
int http_add_header(HttpRequest *req, const char *name, const char *value, MemPool *pool);

/*
 * ヘッダー名に対応する値（raw文字列）を取得する。
 * nameの大小は区別しない（case-insensitive）。
 * 見つからない場合はNULLを返す。
 */
const char *http_get_header(const HttpRequest *req, const char *name);

/* HttpResponse のビルダーAPI */
void http_response_init(HttpResponse *res);
int http_response_set_status(HttpResponse *res, int status_code, const char *reason, MemPool *pool);
int http_response_set_body(HttpResponse *res, const char *body, size_t body_length);
int http_response_add_header(HttpResponse *res, const char *name, const char *value, MemPool *pool);
int http_response_build(HttpResponse *res, MemPool *pool);

#endif /* HTTP_H */
