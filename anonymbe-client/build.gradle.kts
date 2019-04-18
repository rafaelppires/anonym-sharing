import org.jetbrains.kotlin.gradle.tasks.KotlinCompile

plugins {
    id("maven-publish")
    id("com.github.johnrengelman.shadow") version "4.0.3"
    kotlin("jvm") version "1.3.30"
    id("org.jetbrains.kotlin.kapt") version "1.3.30"
    id("me.champeau.gradle.jmh") version "0.4.8"
}

group = "ch.unine"
version = "0.6"

task<Jar>("sourcesJar") {
    from(sourceSets.main.get().allSource)
    archiveClassifier.set("sources")
}

publishing {
    publications {
        create<MavenPublication>("maven") {
            artifactId = "anonymbe-client"
            from(components["java"])
            artifact(tasks["sourcesJar"])
        }
    }
}

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

    kapt("com.squareup.moshi:moshi-kotlin-codegen:1.8.0")

    compile("software.amazon.awssdk:s3:2.5.27")
    compile("software.amazon.awssdk:apache-client:2.5.27")
    compile("org.apache.logging.log4j:log4j-core:2.11.1")
    compile("org.apache.logging.log4j:log4j-api:2.11.1")
    compile("org.apache.logging.log4j:log4j-slf4j-impl:2.11.1")
    compile("io.minio:minio:6.0.4")

    compile("com.squareup.retrofit2:retrofit:2.5.0")
    compile("com.squareup.retrofit2:converter-moshi:2.5.0")
    "com.squareup.okhttp3:okhttp:3.14.1".also {
        compile(it)
        jmh(it)
    }
    /*
    "com.squareup.okhttp3:logging-interceptor:3.12.0".also {
        compile(it)
        jmh(it)
    }
    */
    compile("com.squareup.moshi:moshi:1.8.0")

    testCompile("org.jetbrains.kotlin:kotlin-test-junit:1.3.30")

    jmhCompile("org.openjdk.jmh:jmh-core:1.21")
}

tasks.withType<KotlinCompile> {
    kotlinOptions.jvmTarget = "1.8"
}
