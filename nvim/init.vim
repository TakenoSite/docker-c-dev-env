" 行番号表示
set number
:set autoindent
:set tabstop=4
:set shiftwidth=4
:set smarttab
:set softtabstop=4
:set mouse=a
:set clipboard+=unnamed

" 構文ハイライト
syntax on

" Cファイル用のインデント処理
filetype plugin indent on
"colorscheme desert
colorscheme industry

" プラグイン読み込み
call plug#begin('~/.vim/plugged')
" 補完
Plug 'neoclide/coc.nvim', {'branch': 'release'}
Plug 'https://github.com/preservim/nerdtree' " NerdTree
Plug 'http://github.com/tpope/vim-surround' " Surrounding ysw
Plug 'https://github.com/preservim/nerdtree' " NerdTree
Plug 'https://github.com/tpope/vim-commentary' " For Commenting gcc & gc
Plug 'https://github.com/vim-airline/vim-airline' " Status bar
Plug 'https://github.com/tc50cal/vim-terminal' " Vim Terminal
Plug 'https://github.com/ryanoasis/vim-devicons' " Developer Icons
Plug 'https://github.com/terryma/vim-multiple-cursors' " CTRL + N for multiple cursors
Plug 'https://github.com/preservim/tagbar' " Tagbar for code navigation
Plug 'https://github.com/vim-scripts/ScrollColors' " color scheme

call plug#end()

" coc.nvim用拡張
let g:coc_global_extensions = ['coc-clangd']

" Enterで文字補完する。スニペット展開なし
inoremap <silent><expr> <CR> 
  \ pumvisible() ? coc#_select_confirm() : 
  \ "\<CR>"

" 補完候補が表示されているときに矢印キーで移動
inoremap <expr> <Up>    coc#pum#visible() ? coc#pum#prev(1) : "\<Up>"
inoremap <expr> <Down>  coc#pum#visible() ? coc#pum#next(1) : "\<Down>"

" スニペット展開を完全に無効化
let g:coc_snippet_next = ''
let g:coc_snippet_prev = ''
inoremap <silent> <C-u> <Nop>

noremap <C-f> :NERDTreeFocus<CR>
nnoremap <C-n> :NERDTree<CR>
nnoremap <C-t> :NERDTreeToggle<CR>
