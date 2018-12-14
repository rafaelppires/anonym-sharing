#!/bin/bash

curl --location https://github.com/brianfrankcooper/YCSB/releases/download/0.15.0/ycsb-0.15.0.tar.gz | tar --strip-components=1 --one-top-level=workdir -zxv ycsb-0.15.0/{bin,lib,workloads}

mvn -f YCSB/anonymbe-admin/pom.xml package

patch -p1 < bindings.properties.patch

mkdir -p workdir/anonymbe-binding/lib
cp --reflink=auto YCSB/anonymbe-admin/target/anonymbe-admin-binding-0.15.0.jar YCSB/anonymbe-admin/target/dependency/*.jar workdir/anonymbe-binding/lib/
