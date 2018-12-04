import org.jetbrains.kotlin.gradle.tasks.KotlinCompile

plugins {
    id("com.github.johnrengelman.shadow") version "4.0.3"
    kotlin("jvm") version "1.3.10"
    id("org.jetbrains.kotlin.kapt") version "1.3.10"
    id("me.champeau.gradle.jmh") version "0.4.7"
}

group = "ch.unine"
version = "0.1"

jmh {
    warmupForks = 0
    fork = 1
    threads = 5

    warmup = "5s"
    warmupIterations = 1

    timeOnIteration = "10s"
    iterations = 1

    include = listOf(
        //"AdminBenchmark",
        //"WriterProxyBenchmark"
        "EnvelopeBenchmark"
    )
}

repositories {
    mavenCentral()
}

dependencies {
    compile(kotlin("stdlib-jdk8"))

    kapt("com.squareup.moshi:moshi-kotlin-codegen:1.7.0")

    compile("software.amazon.awssdk:s3:2.1.3")
    compile("software.amazon.awssdk:apache-client:2.1.3")
    compile("org.apache.logging.log4j:log4j-core:2.11.1")
    compile("org.apache.logging.log4j:log4j-api:2.11.1")
    compile("org.apache.logging.log4j:log4j-slf4j-impl:2.11.1")
    compile("io.minio:minio:5.0.2")

    compile("com.squareup.retrofit2:retrofit:2.5.0")
    compile("com.squareup.retrofit2:converter-moshi:2.5.0")
    "com.squareup.okhttp3:okhttp:3.12.0".also {
        compile(it)
        jmh(it)
    }
    /*
    "com.squareup.okhttp3:logging-interceptor:3.12.0".also {
        compile(it)
        jmh(it)
    }
    */
    compile("com.squareup.moshi:moshi:1.7.0")

    testCompile("org.jetbrains.kotlin:kotlin-test-junit:1.3.10")

    jmhCompile("org.openjdk.jmh:jmh-core:1.21")
}

tasks.withType<KotlinCompile> {
    kotlinOptions.jvmTarget = "1.8"
}
