#!/bin/bash

python -m venv ~/venv
source ~/venv/bin/activate

pip install youtube_search yt_dlp argparse
if [ -d "$HOME/venv/lib/python$(python --version | awk '{print $2}' | cut -d. -f1,2)/site-packages" ]; then
	export PYTHONPATH=$HOME/venv/lib/python$(python --version | awk '{print $2}' | cut -d. -f1,2)/site-packages
else
	echo "Error occured, cannot install with quick_install.sh"
	exit 0
fi

deactivate

git clone https://github.com/itrocaiks/yuzutube
cd yuzutube

make
make install
make clean

cp cookies.txt $HOME/.config/yuzutube/conf/cookies.txt

yuzutube --min-width
