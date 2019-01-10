#!/bin/zsh

if [ ! -e LICENSE.txt ]; then
    echo cd to workdir to execute this script
    exit 1
fi

if [ "$#" -ne 1 ]; then
    echo Please specify how much churn is being applied
    exit 1
fi

churn="churn$1"

run=$(date -u +%Y-%m-%d-%H%M)

for filesize in 1k 100k 10m; do
    for workload in workloada workloadb workloadc workloadw; do
        ../init-state.sh bucketonly || exit 1
        bin/ycsb load anonymbe-storage -threads 1 -P ../config.properties -P ../workloads/$workload -P ../workloads/storageworkload -P ../workloads/storage$filesize | tee "storage-$workload-$filesize-$churn-$run.txt" || exit 1
        bin/ycsb run anonymbe-storage -P ../config.properties -P ../workloads/$workload -P ../workloads/storageworkload -P ../workloads/storage$filesize | tee "storage-$workload-$filesize-$churn-$run.txt" || exit 1
    done
done
