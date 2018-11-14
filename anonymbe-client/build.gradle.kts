import org.jetbrains.kotlin.gradle.tasks.KotlinCompile

plugins {
    kotlin("jvm") version "1.3.0"
    id("org.jetbrains.kotlin.kapt") version "1.3.0"
    id("me.champeau.gradle.jmh") version "0.4.7"
}

group = "ch.unine"
version = "0.1"

repositories {
    mavenCentral()
}

dependencies {
    compile(kotlin("stdlib-jdk8"))

    kapt("com.squareup.moshi:moshi-kotlin-codegen:1.7.0")

    compile("software.amazon.awssdk:s3:2.0.0-preview-12")
    compile("io.minio:minio:5.0.2")

    compile("com.squareup.retrofit2:retrofit:2.4.0")
    compile("com.squareup.retrofit2:converter-moshi:2.4.0")
    compile("com.squareup.okhttp3:okhttp:3.11.0")
    compile("com.squareup.moshi:moshi:1.7.0")

    testCompile("org.jetbrains.kotlin:kotlin-test-junit:1.3.0")
}

tasks.withType<KotlinCompile> {
    kotlinOptions.jvmTarget = "1.8"
}
