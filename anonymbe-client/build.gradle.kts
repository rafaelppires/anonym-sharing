import org.jetbrains.kotlin.gradle.tasks.KotlinCompile

plugins {
    id("com.github.johnrengelman.shadow") version "4.0.2"
    kotlin("jvm") version "1.3.10"
    id("org.jetbrains.kotlin.kapt") version "1.3.10"
    id("me.champeau.gradle.jmh") version "0.4.7"
}

group = "ch.unine"
version = "0.1"

jmh {
    warmupForks = 0
    fork = 1
    threads = 1

    warmup = "5s"
    warmupIterations = 1

    timeOnIteration = "10s"
    iterations = 1
}

repositories {
    mavenCentral()
}

dependencies {
    compile(kotlin("stdlib-jdk8"))

    kapt("com.squareup.moshi:moshi-kotlin-codegen:1.7.0")

    compile("software.amazon.awssdk:s3:2.0.0-preview-12")
    compile("io.minio:minio:5.0.2")

    compile("com.squareup.retrofit2:retrofit:2.5.0")
    compile("com.squareup.retrofit2:converter-moshi:2.5.0")
    "com.squareup.okhttp3:okhttp:3.12.0".also {
        compile(it)
        jmh(it)
    }
    compile("com.squareup.moshi:moshi:1.7.0")

    testCompile("org.jetbrains.kotlin:kotlin-test-junit:1.3.10")

    jmh("io.fabric8:kubernetes-client:4.1.0")
}

tasks.withType<KotlinCompile> {
    kotlinOptions.jvmTarget = "1.8"
}
