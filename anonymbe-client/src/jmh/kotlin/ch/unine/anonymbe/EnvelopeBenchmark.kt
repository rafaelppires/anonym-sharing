package ch.unine.anonymbe

import ch.unine.anonymbe.api.*
import org.openjdk.jmh.annotations.*
import java.util.concurrent.TimeUnit

@State(Scope.Benchmark)
@OutputTimeUnit(TimeUnit.MILLISECONDS)
open class EnvelopeBenchmark {
    @Param("1", "10", "100", "1000", "10000")
    private var groupSize: Int = 0

    @Param("1")
    private var scale: String = ""

    private lateinit var userService: UserApi

    @Setup(Level.Trial)
    fun setup() {
        Cluster.scaleService(Deployment.ANONYMBE_MONGO, scale.toInt())

        userService = Api.build(Deployments.ANONYMBE_MONGO_URL)
        val necessaryUserGroups = arrayOf(
            UserGroup("envelope-user-1", "envelope-group-$groupSize"),
            UserGroup("envelope-user-$groupSize", "envelope-group-$groupSize")
        )

        for (userGroup in necessaryUserGroups) {
            if (!userService.isUserPartOfGroup(userGroup).execute().bodyOrElseThrow.belongs) {
                throw IllegalStateException("Database does not contain the right data")
            }
        }
    }

    @TearDown(Level.Trial)
    fun tearDown() {
        Cluster.scaleService(Deployment.ANONYMBE_MONGO, 0)
    }

    @Benchmark
    @BenchmarkMode(Mode.Throughput)
    @OutputTimeUnit(TimeUnit.SECONDS)
    fun createEnvelopeBenchmarkTput() {
        createEnvelopeBenchmark()
    }

    @Benchmark
    @BenchmarkMode(Mode.AverageTime)
    @OutputTimeUnit(TimeUnit.MILLISECONDS)
    fun createEnvelopeBenchmarkAvgt() {
        createEnvelopeBenchmark()
    }

    @Suppress("NOTHING_TO_INLINE")
    private inline fun createEnvelopeBenchmark() {
        val envelope = userService.getEnvelope(
            Bucket("envelope-user-$groupSize", "envelope-group-$groupSize", BUCKET_KEY)
        ).execute()
        envelope.throwExceptionIfNotSuccessful()

        if (envelope.body()?.ciphertext.isNullOrEmpty()) {
            throw IllegalStateException("ciphertext is empty")
        }
    }

    companion object {
        const val BUCKET_KEY = "ag7MS7ENVNo3ftj7GPjk+NN6sCpGqZLJbOwvjCCj+uc="
    }
}
