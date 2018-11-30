package ch.unine.anonymbe

import ch.unine.anonymbe.api.AdminApi
import ch.unine.anonymbe.api.Api
import ch.unine.anonymbe.api.User
import ch.unine.anonymbe.api.throwExceptionIfNotReallySuccessful
import org.openjdk.jmh.annotations.*
import org.openjdk.jmh.infra.ThreadParams

@State(Scope.Thread)
open class CounterThreadState {
    var counter = 0

    @Setup(Level.Trial)
    fun setup(params: ThreadParams) {
        counter = params.threadIndex * (Int.MAX_VALUE / params.threadCount)
    }
}

@State(Scope.Benchmark)
open class AdminBenchmark {
    @Param(Deployments.ANONYMBE_MEM_URL, Deployments.ANONYMBE_MONGO_URL)
    private var endpointUrl: String = ""

    @Param("1", "2")
    private var scale: String = ""

    private lateinit var service: AdminApi

    @Setup(Level.Trial)
    fun setup(ts: CounterThreadState) {
        Cluster.scaleService(Deployment.fromUrl(endpointUrl), scale.toInt())

        service = Api.build(endpointUrl)
        var tries = 0
        while (tries < 0 && try {
                service.deleteAllData().execute().throwExceptionIfNotReallySuccessful()
                false
            } catch (e: Exception) {
                true
            }
        ) {
            tries++
            println("Error deleting all data, waiting 1 second")
            Thread.sleep(1000)
        }
    }

    @Benchmark
    fun addUserBenchmark(ts: CounterThreadState) {
        val name = "testuser${++ts.counter}"
        val user = User(name)
        service.createUser(user).execute()
    }
}
