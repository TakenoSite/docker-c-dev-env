# C Development Environment - Docker

Ubuntu 22.04をベースとした開発環境のDockerイメージです。C言語開発に必要なツール・ライブラリが予め設定されています。

## Dockerfile 設定詳細

### ベースイメージ
- **Ubuntu 22.04**: 安定性と豊富なパッケージリポジトリが特徴

### インストール済みツール・ライブラリ

#### システム構成
- **シェル**: Zsh + Oh-my-zsh + Powerlevel10k テーマ
- **デフォルトシェル**: Zsh（rootユーザー）
- **ロケール**: en_US.UTF-8
- **ターミナル**: xterm-256color

#### 開発ツール
- **C/C++**: clang, clang-d, build-essential, cmake
- **デバッガ**: gdb
- **バージョン管理**: git, GitHub CLI (gh)
- **エディタ**: Neovim (0.8以上) + vim-plug
  - coc.nvimプラグイン自動インストール
  - coc-settings.json で LSP/補完機能設定

#### プログラミング言語
- **Python3**: python3, python3-pip
- **Node.js**: 18系（公式スクリプトからインストール）

#### ネットワーク・セキュリティ
- OpenSSH Server
  - デフォルトユーザー: root
  - デフォルトパスワード: taketo
  - PermitRootLogin: enabled (yes)
- **ポート**: SSH (22)
- nmap, net-tools: ネットワーク診断ツール

#### Webサーバー
- Apache2

#### その他
- curl, wget: ファイルダウンロード
- ca-certificates, gnupg: SSL/暗号化
- unzip: アーカイブ解凍
- htop: プロセスモニタリング
- xsel, xclip: クリップボード機能
- 日本語フォント（noto-cjk, ipafont, unfonts）

### 設定ファイル
以下のファイルがイメージに統合されます：
```
zsh/.zshrc              → /root/.zshrc (Zsh設定)
zsh/.p10k.zsh           → /root/.p10k.zsh (Powerlevel10k設定)
tmux/tmux.conf          → /root/.tmux.conf (Tmux設定)
nvim/init.vim           → /root/.config/nvim/init.vim (Neovim設定)
nvim/coc-settings.json  → /root/.config/nvim/coc-settings.json (LSP設定)
```

### ワークディレクトリ
- `/workspace`: コンテナ内のアクティブディレクトリ

## 使い方

### 1. イメージビルド
```bash
make build
```
または
```bash
docker build -t c-dev-env .
```

### 2. コンテナ実行

#### デフォルトポートで実行（推奨）
SSH (22) と UDP 12345 を公開します：
```bash
make run
```

#### カスタムポート付きで実行
追加ポートを公開したい場合：
```bash
make run-custom PORTS="-p 8080:8080 -p 3000:3000"
```

複数ポートを指定する場合：
```bash
make run-custom PORTS="-p 8080:8080 -p 9000:9000/UDP -p 5000:5000"
```

#### 直接実行（Makefileなし）
```bash
docker run -it --rm \
  -p 22:22 \
  -p 12345:12345/UDP \
  -v "${PWD}:/workspace" \
  c-dev-env
```

### 3. SSHでの接続
コンテナがバックグラウンド実行の場合：
```bash
ssh root@localhost
```
パスワード: `taketo`

### 4. インタラクティブセッションを終了
```bash
exit
```

## ポート設定

| ポート | プロトコル | 用途 | デフォルト |
|--------|-----------|------|----------|
| 22 | TCP | SSH | ✅ |
| 12345 | UDP | カスタムアプリケーション | ✅ |
| 8080 | TCP | Webアプリ（オプション） | ❌ |

## ホームディレクトリ構成
```
/root/
├── .zshrc               # Zsh設定
├── .p10k.zsh            # Powerlevel10k設定
├── .tmux.conf           # Tmux設定
├── .config/nvim/
│   ├── init.vim         # Neovim初期化スクリプト
│   ├── coc-settings.json# LSP/補完設定
│   └── autoload/
│       └── plug.vim     # vim-plug（プラグインマネージャ）
└── .oh-my-zsh/
    └── custom/themes/powerlevel10k/  # テーマ
```

## トラブルシューティング

### SSH接続できない
- コンテナが実行中か確認: `docker ps`
- ポート22が正しくマッピングされているか確認: `docker port <container_id>`

### Neovimでプラグインが読み込まれない
- コンテナ内で実行:
  ```bash
  nvim --headless +PlugInstall +qall
  ```

### zshが起動しない
- bashで起動して設定を確認:
  ```bash
  docker run -it --rm -v "${PWD}:/workspace" c-dev-env /bin/bash
  ```

## カスタマイズ

各設定ファイルを編集してからビルドしてください：
- `zsh/.zshrc`: Zshエイリアス・関数など
- `nvim/init.vim`: Neovimプラグイン・キーマッピング
- `Dockerfile`: インストール対象を追加削除
