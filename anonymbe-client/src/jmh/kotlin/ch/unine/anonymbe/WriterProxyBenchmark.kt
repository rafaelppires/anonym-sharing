package ch.unine.anonymbe

import ch.unine.anonymbe.storage.Minio
import ch.unine.anonymbe.storage.StorageApi
import ch.unine.anonymbe.storage.WriterProxy
import org.openjdk.jmh.annotations.*
import java.util.*
import kotlin.random.Random

@State(Scope.Benchmark)
open class WriterProxyBenchmark {
    @Param("true")//, "false")
    private var isThroughProxyString: String = ""

    @Param("10000")
    private var dataSizeBytes: String = ""

    @Param("1", "2", "3", "4")
    private var scale: String = ""

    @Volatile
    private var errors: Int = 0

    private lateinit var service: StorageApi

    private val bucket = UUID.randomUUID().toString()
    private lateinit var data: ByteArray

    @Setup(Level.Trial)
    fun setup() {
        service = when (isThroughProxyString) {
            "true" -> {
                Cluster.scaleService(Deployment.WRITERPROXY, scale.toInt())
                WriterProxy(writerProxyUrl = Deployments.WRITERPROXY_URL, minioUrl = Deployments.MINIO_URL)
            }
            "false" -> {
                Cluster.scaleService(Deployment.WRITERPROXY, 0)
                Minio(endpoint = Deployments.MINIO_URL)
            }
            else -> throw IllegalArgumentException()
        }
        println("Bucket = $bucket")
        service.createBucketIfNotExists(bucket)
        data = Base64.getEncoder().encode(Random.nextBytes(dataSizeBytes.toInt()))
    }

    @TearDown
    fun tearDown() {
        println("Number of errors: $errors")

        service.deleteBucket(bucket)
    }

    @Benchmark
    fun uploadFile() {
        val objectName = UUID.randomUUID().toString()
        try {
            service.storeObject(bucket, objectName, data.inputStream())
        } catch (_: Exception) {
            errors++
        }

    }
}