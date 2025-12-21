# c-tiny-http_parser

C11製のシンプルなHTTPリクエスト/レスポンス処理ライブラリとテスト群です。`mm_pool`によるリニアメモリプールを使い、断片化を抑えつつ文字列を扱います。

## ファイル構成
- http.h / http.c: リクエストパース、リクエスト/レスポンスのビルダーAPI、レスポンスシリアライズ
- mm_pool.h / mm_pool.c: 固定サイズチャンクを連結したリニアメモリプール
- test_http.c: リクエストパースとレスポンスビルドのテスト
- test_mm_pool.c: メモリプールのテスト
- makefile: ビルド設定

## ビルドとテスト
```sh
make           # テストバイナリ(test_http, test_mm_pool)を生成
./test_http    # HTTP処理の動作確認
./test_mm_pool # メモリプールの動作確認
make clean     # 生成物削除
```

## 使い方
### メモリプール
全APIは`MemPool`上に文字列を確保します。使い終わったら`mm_pool_destroy`でまとめて解放できます。
```c
MemPool *pool = mm_pool_create(2048, 1);
// ... use the pool ...
mm_pool_destroy(pool);
```

### リクエストをパースする
```c
const char *raw = "POST /submit HTTP/1.1\r\n"
                  "Host: example.com\r\n"
                  "Content-Type: text/plain\r\n"
                  "Content-Length: 5\r\n"
                  "\r\n"
                  "hello";
HttpRequest req;
int rc = http_parse_request(raw, strlen(raw), &req, pool);
assert(rc == 0);
// ヘッダー取得（大小区別なし）
const char *ct = http_get_header(&req, "content-type");
// ボディ参照
assert(req.body_length == 5 && strncmp(req.body, "hello", 5) == 0);
```

### リクエストを組み立てる（ビルダー）
```c
HttpRequest req;
http_request_init(&req);
http_set_request_line(&req, "GET", "/", "HTTP/1.1", pool);
http_add_header(&req, "Host", "example.com", pool);
```
ヘッダー値は`,`や`;`でトークン分割され、`HttpHeaderValue.items`に格納されます。

### レスポンスを組み立ててシリアライズする
```c
HttpResponse res;
http_response_init(&res);
http_response_set_status(&res, 200, "OK", pool);
http_response_add_header(&res, "Content-Type", "text/plain", pool);
const char body[] = "hi";
http_response_set_body(&res, body, strlen(body));
http_response_add_header(&res, "Content-Length", "2", pool);
http_response_build(&res, pool);
// res.raw / res.raw_length に完全なHTTPレスポンス文字列が入る
```

## 制約・メモ
- チャンクを超えるサイズはプールが追加チャンクを確保して処理します。
- メモリプールは個別解放を持たず、`mm_pool_reset`または`mm_pool_destroy`でまとめて管理します。
