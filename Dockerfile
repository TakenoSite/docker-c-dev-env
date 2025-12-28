FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV LANG=C.UTF-8
ENV TERM=xterm-256color

# システム基本パッケージ
RUN apt-get update && apt-get install -y \
    software-properties-common \
    ca-certificates \
    gnupg \
    lsb-release \
    locales

# ファイル・ダウンロードツール
RUN apt-get install -y \
    curl \
    wget \
    unzip

# バージョン管理・ターミナル
RUN apt-get install -y \
    git \
    tmux \
    zsh

# C/C++ 開発ツール
RUN apt-get install -y \
    build-essential \
    clang \
    clangd \
    cmake \
    libssl-dev \
    gdb

# ARM系クロスコンパイラ（32-bit: arm-linux-gnueabihf, 64-bit: aarch64-linux-gnu）
RUN apt-get install -y \
    gcc-arm-linux-gnueabihf \
    g++-arm-linux-gnueabihf \
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    binutils-arm-linux-gnueabihf \
    binutils-aarch64-linux-gnu

# Python 開発環境
RUN apt-get install -y \
    python3 \
    python3-pip

# ネットワーク診断ツール
RUN apt-get install -y \
    nmap \
    net-tools

# Webサーバー
RUN apt-get install -y \
    apache2

# ユーティリティ
RUN apt-get install -y \
    htop \
    xsel \
    xclip

# フォント（UI・日本語対応）
RUN apt-get install -y \
    fonts-powerline \
    fonts-noto-cjk \
    fonts-ipafont \
    fonts-unfonts-core

RUN apt install -y openssh-server \
 && mkdir /var/run/sshd \
 && echo 'root:taketo' | chpasswd \
 && sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config

# ロケール設定
RUN locale-gen en_US.UTF-8

# Neovimの古い版を削除してPPAから最新版インストール（0.8以上保証）
RUN apt-get remove -y neovim
RUN add-apt-repository ppa:neovim-ppa/unstable -y
RUN apt-get update && apt-get install -y neovim

# 文字コードを統一
RUN apt-get update && apt-get install -y locales && \
    locale-gen en_US.UTF-8 && \
    update-locale LANG=en_US.UTF-8

ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US:en
ENV LC_ALL=en_US.UTF-8

# Node.js 18系を公式スクリプトからインストール
RUN curl -fsSL https://deb.nodesource.com/setup_18.x | bash - && \
    apt-get install -y nodejs

# GitHub CLIインストール
RUN curl -fsSL https://cli.github.com/packages/githubcli-archive-keyring.gpg | \
    gpg --dearmor -o /usr/share/keyrings/githubcli-archive-keyring.gpg && \
    echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" | \
    tee /etc/apt/sources.list.d/github-cli.list > /dev/null && \
    apt-get update && apt-get install -y gh

# oh-my-zshインストール + powerlevel10kテーマ
RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)" "" --unattended && \
    git clone --depth=1 https://github.com/romkatv/powerlevel10k.git /root/.oh-my-zsh/custom/themes/powerlevel10k

# zsh をデフォルトシェルに設定
RUN chsh -s $(which zsh) root

# vim-plug (Neovimプラグインマネージャ) のインストール
RUN curl -fLo /root/.local/share/nvim/site/autoload/plug.vim --create-dirs \
    https://raw.githubusercontent.com/junegunn/vim-plug/master/plug.vim

# neovim用設定ディレクトリ作成
RUN mkdir -p /root/.config/nvim

# zsh 設定ファイルをコピー（要準備、下記参照）
COPY zsh/.zshrc /root/.zshrc
COPY zsh/.p10k.zsh /root/.p10k.zsh

# tmux設定コピー
COPY tmux/tmux.conf /root/.tmux.conf

# nvim 設定ファイルをコピー（要準備、下記参照）
COPY nvim/init.vim /root/.config/nvim/init.vim
COPY nvim/coc-settings.json /root/.config/nvim/coc-settings.json

# Neovimのプラグインをインストール（coc.nvimなど）
RUN nvim --headless +PlugInstall +qall || true

WORKDIR /workspace

# 任意コマンドを設定する
RUN echo "alias cls='clear'" >> /root/.zshrc
RUN echo "service ssh start" >> /root/.zshrc
#RUN echo "tmux" >> /root/.zshrc
RUN echo "clear" >> /root/.zshrc

# zshを実行
CMD ["zsh"]
