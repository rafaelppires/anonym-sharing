package ch.unine.anonymbe

import ch.unine.anonymbe.api.*
import org.openjdk.jmh.annotations.*
import org.openjdk.jmh.infra.ThreadParams

@State(Scope.Benchmark)
open class EnvelopeBenchmark : AdminBenchmark() {
    @Param("1", "10", "100", "1000", "10000")
    private var groupSize: Int = 0

    private lateinit var userService: UserApi

    @Setup(Level.Trial)
    override fun setup() {
        super.setup()
        userService = Api.build()
    }

    @Benchmark
    @BenchmarkMode(Mode.AverageTime)
    fun createEnvelopeBenchmark() {
        try {
            userService.getEnvelope(
                Bucket("envelope-user-$groupSize", "envelope-group-$groupSize", BUCKET_KEY)
            ).execute()
        } catch (_: Exception) {
            errors++
        }
    }

    companion object {
        const val BUCKET_KEY = "ag7MS7ENVNo3ftj7GPjk+NN6sCpGqZLJbOwvjCCj+uc="
    }
}
