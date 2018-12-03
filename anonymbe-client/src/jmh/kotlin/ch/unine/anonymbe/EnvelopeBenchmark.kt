package ch.unine.anonymbe

import ch.unine.anonymbe.api.Api
import ch.unine.anonymbe.api.Bucket
import ch.unine.anonymbe.api.UserApi
import ch.unine.anonymbe.api.throwExceptionIfNotReallySuccessful
import org.openjdk.jmh.annotations.*
import java.util.concurrent.TimeUnit

@State(Scope.Benchmark)
@OutputTimeUnit(TimeUnit.MILLISECONDS)
open class EnvelopeBenchmark {
    @Param("1", "10", "100", "1000", "10000")
    private var groupSize: Int = 0

    @Param("1", "2")
    private var scale: String = ""

    private lateinit var userService: UserApi

    @Setup(Level.Trial)
    fun setup() {
        Cluster.scaleService(Deployment.ANONYMBE_MONGO, scale.toInt())

        userService = Api.build()
    }

    @TearDown(Level.Trial)
    fun tearDown() {
        Cluster.scaleService(Deployment.ANONYMBE_MONGO, 0)
    }

    @Benchmark
    @BenchmarkMode(Mode.AverageTime)
    fun createEnvelopeBenchmark() {
        userService.getEnvelope(
            Bucket("envelope-user-$groupSize", "envelope-group-$groupSize", BUCKET_KEY)
        ).execute().throwExceptionIfNotReallySuccessful()
    }

    companion object {
        const val BUCKET_KEY = "ag7MS7ENVNo3ftj7GPjk+NN6sCpGqZLJbOwvjCCj+uc="
    }
}
