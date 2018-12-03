package ch.unine.anonymbe

import ch.unine.anonymbe.api.AdminApi
import ch.unine.anonymbe.api.Api
import ch.unine.anonymbe.api.throwExceptionIfNotReallySuccessful
import org.openjdk.jmh.annotations.*

@State(Scope.Benchmark)
abstract class AdminBenchmark {
    @Param(Deployments.ANONYMBE_MONGO_URL) //Deployments.ANONYMBE_MEM_URL
    private var endpointUrl: String = ""

    @Param("1", "2")
    private var scale: String = ""

    @Volatile
    protected var errors = 0

    protected lateinit var service: AdminApi

    @Setup(Level.Trial)
    open fun setup() {
        Cluster.scaleService(Deployment.fromUrl(endpointUrl), scale.toInt())

        service = Api.build(endpointUrl)
    }

    @TearDown(Level.Trial)
    fun tearDown() {
        Cluster.scaleService(Deployment.fromUrl(endpointUrl), 0)
    }

    @TearDown(Level.Iteration)
    fun iterationTearDown() {
        println("Number of errors: $errors")
        errors = 0

        var tries = 10
        while (tries --> 0 && try {
                service.deleteAllData().execute().throwExceptionIfNotReallySuccessful()
                false
            } catch (e: Exception) {
                true
            }
        ) {
            println("Error deleting all data, waiting 1 second")
            Thread.sleep(1000)
        }
    }
}
