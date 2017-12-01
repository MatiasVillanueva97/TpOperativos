#!/bin/bash

echo "Instalando COMMONS..."
git clone https://github.com/sisoputnfrba/so-commons-library.git 
cd so-commons-library
sudo make install

echo "Instalando READLINE..."
sudo apt-get install -y libreadline6-dev

set -o allexport
echo "export LC_ALL=C" > ./export
. ./export
set +o allexport

#echo "Bajando el repo..."
#curl -u 'nahueloyha' -L -o tp-2017-2c-Mi-Grupo-1234.tar https://api.github.com/repos/sisoputnfrba/tp-2017-2c-Mi-Grupo-1234/tarball/master
#mkdir tp-2017-2c-Mi-Grupo-1234
#tar -xvf tp-2017-2c-Mi-Grupo-1234.tar --strip 1 -C tp-2017-2c-Mi-Grupo-1234 

