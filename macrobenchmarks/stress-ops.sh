#!/bin/zsh

if [ ! -e LICENSE.txt ]; then
    echo cd to workdir to execute this script
    exit 1
fi

if [ "$#" -ne 1 ]; then
    echo Please specify how much churn is being applied
    exit 1
fi

churn="$1"

../init-state.sh || exit 1
bin/ycsb load anonymbe-admin -P ../config.properties -P ../workloads/adminworkload -threads 4 || exit 1
bin/ycsb run anonymbe-admin -P ../config.properties -P ../workloads/adminworkload -threads 4 -target $churn -s || exit 1
