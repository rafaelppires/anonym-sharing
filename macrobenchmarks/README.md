# How to setup the macro-benchmarks

**Requirements:**

* Java **11** (beware of Ubuntu's fake `openjdk-11` package which contain Java 10)
* Maven (`mvn` command)
* curl
* Predictable user keys in the AnonymBE service (look for git branch)

**Steps:**

1. Execute `./setup.sh`
2. Go in `workdir`
3. Execute `bin/ycsb`
