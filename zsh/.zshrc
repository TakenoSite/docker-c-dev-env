export ZSH="$HOME/.oh-my-zsh"
ZSH_THEME="powerlevel10k/powerlevel10k"
plugins=(git)
source $ZSH/oh-my-zsh.sh

# ヒストリやカラフルな出力強化（Kali風）
export HISTFILE=~/.zsh_history
export HISTSIZE=10000
export SAVEHIST=10000

alias ls='ls --color=auto'
alias ll='ls -alF'
alias la='ls -A'

[[ ! -f ~/.p10k.zsh ]] || source ~/.p10k.zsh
