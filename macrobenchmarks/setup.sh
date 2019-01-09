#!/bin/bash

if [ ! -e workdir/LICENSE.txt ]; then
    curl --location https://github.com/brianfrankcooper/YCSB/releases/download/0.15.0/ycsb-0.15.0.tar.gz | tar --strip-components=1 --one-top-level=workdir -zxv ycsb-0.15.0/{bin,lib,workloads,LICENSE.txt,NOTICE.txt}
fi

../anonymbe-client/gradlew --no-daemon -p ../anonymbe-client publishToMavenLocal || exit -1

mvn -f YCSB/anonymbe-admin/pom.xml package
mvn -f YCSB/anonymbe-storage/pom.xml package

patch -p1 < bindings.properties.patch
patch -p1 < ycsb.patch

mkdir -p workdir/anonymbe-{admin,storage}-binding/lib
cp -n --reflink=auto YCSB/anonymbe-admin/target/anonymbe-admin-binding-0.15.0.jar YCSB/anonymbe-admin/target/dependency/*.jar workdir/anonymbe-admin-binding/lib/
cp -n --reflink=auto YCSB/anonymbe-storage/target/anonymbe-storage-binding-0.15.0.jar YCSB/anonymbe-storage/target/dependency/*.jar workdir/anonymbe-storage-binding/lib/
