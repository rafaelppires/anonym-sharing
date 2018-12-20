package ch.unine.anonymbe

import ch.unine.anonymbe.storage.HybridTokenAwsMinio
import ch.unine.anonymbe.storage.Minio
import ch.unine.anonymbe.storage.StorageApi
import ch.unine.anonymbe.storage.WriterProxy
import org.openjdk.jmh.annotations.*
import java.util.*
import kotlin.random.Random

@State(Scope.Benchmark)
open class WriterProxyBenchmark {
    @Param("true", "false")
    private var isThroughProxyString: String = ""

    @Param("true", "false")
    private var isTokenAccess: String = ""

    @Param("1024", "10240", "102400", "1024000")
    private var dataSizeBytes: String = ""

    @Param("1", "2", "3", "4", "5", "6")
    private var scale: String = ""

    @Volatile
    private var errors: Int = 0

    private lateinit var service: StorageApi

    private val bucket = UUID.randomUUID().toString()
    private lateinit var data: ByteArray

    @Setup(Level.Trial)
    fun setup() {
        service = when {
            isThroughProxyString == "true" && isTokenAccess == "false" -> {
                val scale = scale.toInt()
                assert(scale > 0)
                Cluster.scaleService(Deployment.WRITERPROXY, scale)
                Cluster.scaleService(Deployment.WRITERPROXY_TOKEN, 0)
                Cluster.scaleService(Deployment.NGINX, 0)
                WriterProxy(Minio(Deployments.MINIO_URL), Deployments.WRITERPROXY_URL)
            }
            isThroughProxyString == "false" && isTokenAccess == "false" -> {
                Cluster.scaleService(Deployment.WRITERPROXY, 0)
                Cluster.scaleService(Deployment.WRITERPROXY_TOKEN, 0)
                Cluster.scaleService(Deployment.NGINX, 0)
                Minio(Deployments.MINIO_URL)
            }
            isThroughProxyString == "false" && isTokenAccess == "true" -> {
                val scale = scale.toInt()
                assert(scale > 0)
                Cluster.scaleService(Deployment.WRITERPROXY, 0)
                Cluster.scaleService(Deployment.WRITERPROXY_TOKEN, scale)
                Cluster.scaleService(Deployment.NGINX, 1)
                HybridTokenAwsMinio(
                    minioEndpoint = Deployments.NGINX_URL,
                    writerProxyEndpoint = Deployments.WRITERPROXY_TOKEN_URL
                )
            }
            else -> throw IllegalArgumentException()
        }
        println("Bucket = $bucket")

        data = Base64.getEncoder().encode(Random.nextBytes(dataSizeBytes.toInt()))
    }

    @Setup(Level.Iteration)
    fun iterationSetup() {
        service.createBucketIfNotExists(bucket)
    }

    @TearDown(Level.Trial)
    fun tearDown() {
        Cluster.scaleService(Deployment.WRITERPROXY, 0)
        Cluster.scaleService(Deployment.WRITERPROXY_TOKEN, 0)
        Cluster.scaleService(Deployment.NGINX, 1)
    }

    @TearDown(Level.Iteration)
    fun iterationTearDown() {
        println("Number of errors: $errors")

        println("")
        service.deleteBucket(bucket)
    }

    @Benchmark
    fun uploadFile() {
        val objectName = UUID.randomUUID().toString()
        try {
            service.storeObject(bucket, objectName, data)
        } catch (_: Exception) {
            errors++
        }

    }
}

