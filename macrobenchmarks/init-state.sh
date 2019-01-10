#!/bin/zsh

if [ ! -e LICENSE.txt ]; then
    echo "Please cd to workdir before executing this script"
    exit 1
fi

noglob java -cp anonymbe-binding/lib/* ch.unine.anonymbe.InitMacrobenchmarksKt "$@"
