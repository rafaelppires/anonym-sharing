package ch.unine.anonymbe

import ch.unine.anonymbe.api.AdminApi
import ch.unine.anonymbe.api.Api
import ch.unine.anonymbe.api.User
import ch.unine.anonymbe.api.throwExceptionIfNotReallySuccessful
import org.openjdk.jmh.annotations.*
import org.openjdk.jmh.infra.BenchmarkParams
import org.openjdk.jmh.infra.ThreadParams
import retrofit2.Retrofit

@State(Scope.Thread)
open class ThreadState {
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
    private var scaleString: String = ""

    private lateinit var service: AdminApi

    @Setup(Level.Trial)
    fun setup(ts: ThreadState) {
        service = Api.build(endpointUrl)
        service.deleteAllData().execute().throwExceptionIfNotReallySuccessful()

        val scale = scaleString.toInt()
        Cluster.scaleAnonymBEService(Deployment.fromUrl(endpointUrl), scale)
    }

    @TearDown(Level.Trial)
    fun tearDown() {
        Api.okHttpClient.connectionPool().evictAll()
    }

    @Benchmark
    fun addUserBenchmark(ts: ThreadState) {
        val name = "testuser${++ts.counter}"
        val user = User(name)
        service.createUser(user).execute()
    }

    @Benchmark
    fun addUserToGroupBenchmark() {

    }
}
