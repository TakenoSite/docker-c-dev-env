.PHONY: build run help

# デフォルトのポート指定
DEFAULT_PORTS = -p 22:22 -p 12345:12345/UDP

# ビルド
build:
	docker build -t c-dev-env .

# デフォルトポート（22:22, 12345:12345/UDP）で実行
run: build
	docker run -it --rm -v "${PWD}:/workspace" $(DEFAULT_PORTS) -d c-dev-env

# カスタムポート付きで実行（例: make run-custom PORTS="-p 8080:8080 -p 3000:3000"）
run-custom: build
	docker run -it --rm -v "${PWD}:/workspace" $(DEFAULT_PORTS) $(PORTS) -d c-dev-env

# ヘルプ
help:
	@echo "使用方法:"
	@echo "  make build              - Dockerイメージをビルド"
	@echo "  make run                - デフォルトポート(22:22, 12345:12345/UDP)で実行"
	@echo "  make run-custom PORTS=\"-p 8080:8080\" - カスタムポート付きで実行"
