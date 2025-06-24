" 行番号表示
set number

" 構文ハイライト
syntax on

" Cファイル用のインデント処理
filetype plugin indent on

" プラグイン読み込み
call plug#begin('~/.vim/plugged')

" 補完
Plug 'neoclide/coc.nvim', {'branch': 'release'}

" ファイルツリー
Plug 'nvim-tree/nvim-tree.lua'
Plug 'nvim-tree/nvim-web-devicons'

call plug#end()

" coc.nvim用拡張
let g:coc_global_extensions = ['coc-clangd']

" F2 でファイルツリー切替
nnoremap <F2> :NvimTreeToggle<CR>
