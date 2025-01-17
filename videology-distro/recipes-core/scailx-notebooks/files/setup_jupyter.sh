#! /usr/bin/env bash

[ -f /opt/jupter/work/bin/jupyter-lab ] && exit 0 

mkdir -p /opt/jupyter/env /opt/jupyter/notebooks
virtualenv /opt/jupyter/env --system-site-packages

/opt/jupyter/env/bin/python3 -m pip install wheel
/opt/jupyter/env/bin/python3 -m pip install jupyterlab jupyterlab-lsp
/opt/jupyter/env/bin/python3 -m pip install ipywidgets
/opt/jupyter/env/bin/python3 -m pip install 'python-lsp-server[all]'

cd /opt/jupyter/env

