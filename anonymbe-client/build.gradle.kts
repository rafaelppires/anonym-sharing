import org.jetbrains.kotlin.gradle.tasks.KotlinCompile

plugins {
    id("org.jetbrains.kotlin.kapt") version "1.3.0"
    kotlin("jvm") version "1.3.0"
}

group = "ch.unine"
version = "0.1"

repositories {
    mavenCentral()
}

dependencies {
    kapt("com.squareup.moshi:moshi-kotlin-codegen:1.7.0")
    compile(kotlin("stdlib-jdk8"))

    compile("com.squareup.retrofit2:retrofit:2.4.0")
    compile("com.squareup.retrofit2:converter-moshi:2.4.0")
    compile("com.squareup.okhttp3:okhttp:3.11.0")
    implementation("com.squareup.moshi:moshi:1.7.0")
}

tasks.withType<KotlinCompile> {
    kotlinOptions.jvmTarget = "1.8"
}
