# yuzutube
a TUI software based on youtube_search and yt-dlp python modules (and cJSON, ncurses libraries) for Linux

<p align="center">
  <img src="./pics/working_term.gif" width="400">
</p>

## Installation
### Installing python, cjson, ncurses
Firstly you have to install python, cjson, ncurses, clang(or gcc) from your package manager
for pacman it's:
```
sudo pacman -Syu python cjson ncurses clang
```
### Getting cookies.txt
you will have to get your youtube cookies (use chrome extension for it: https://chromewebstore.google.com/search/cookies.txt)
<br>
After getting cookies file rename it to cookies.txt
```
mv <your_cookie_file> cookies.txt
```
<br>
After this, you may run quick_install.sh, which installs and configures everything by default,<br>
skip this part if you want to install it manually

```
git clone https://github.com/itrocaiks/yuzutube
mv cookies.txt yuzutube/
cd yuzutube
/bin/bash quick_install.py
```
###
if you're having issues with installation, please report in "Issues"
### Installing python modules
Ensure that you have pip modules youtube_search, yt_dlp, argparse
<br><br>Creating and entering venv (if you're using fish, use <path/to/venv/bin/activate.fish>) (to quit enter 'deactivate')
```
python -m venv <path/to/venv>
source <path/to/venv/bin/activate>
```
<br>
Installing modules

```
pip install youtube_search yt_dlp argparse
```
<br>
Set a $PYTHONPATH environmental (needed when running a compiled software)
<br>
also, it will be unset after exiting your shell, to set it the permanently write this command to your .bashrc/.zshrc (depends on shell)

```
export PYTHONPATH=$HOME/venv/lib/python$(python --version | awk '{print $2}' | cut -d. -f1,2)/site-packages
```
### Copying git repo and installing with make
Also make sure you have git installed (from your package manager)
```
git clone https://github.com/itrocaiks/yuzutube
cd yuzutube
```

<br>
Compiling and installing with make (If you want to uninstall run 'make uninstall')

```
make
make clean
make install
```

## Configuration
### Setting envs
you have to set next environmental variables:
```
# optional, limit for results, default: 10
export YUZU_MAX_LIMIT=20
```
```
# optional, default: '$HOME/.config/yuzutube', if you ran 'make install', your config path is '$HOME/.config/yuzutube'
export YUZU_CONFIG_PATH=<your_config_path_here>
```
```
# optional, default: '$PWD' (this directory), dir where your downloaded media will be stored
export YUZU_DOWNLOAD_DIR=<your_download_dir_here>
```
```
# required if not running in venv, dir depends on your venv
export PYTHONPATH=$HOME/venv/lib/python3.13/site-packages 
```
!!! NOTICE THAT ALL THESE ENVIRONMENTAL VARIABLES WILL BE UNSET AFTER EXITING !!!
####
(to set it the permanently write these commands to your .bashrc/.zshrc (depends on shell))

### Getting cookies.txt (if you didn't get it before)
you will have to get your youtube cookies (use chrome extension for it: https://chromewebstore.google.com/search/cookies.txt)
<br>
After getting cookies file, copy it to conf dir:
```
cp <your_cookie_file> $HOME/.config/yuzutube/conf/cookies.txt
```

if you have set the YUZU_CONFIG_PATH:
```
cp <your_cookie_file> $YUZU_CONFIG_PATH/conf/cookies.txt
```

## Runing yuzutube
after installing and configuring everything just as it was said here, you can run yuzutube.

#### Usage:
```
 Usage: yuzutube [OPTION]
	--min-width    allows terminal width < 60
	--help         display this message
```
#### Example
```
yuzutube
```

the interface is user-friendly, to quit from searching line, press Ctrl+x, to quit from anywhere, press Ctrl+c<br>
to search, select video or format, press Enter from corresponding window<br>
to interrupt download or get back to inputting press 'q'

## Afterwords
i've been making this TUI for 19 hours/day, i got a headache, and it's not very likely that I'd maintain this software any furtherr<br>
but it would be nice if you report something in issues, i'll try to help you as soon as i receive the notification :3




