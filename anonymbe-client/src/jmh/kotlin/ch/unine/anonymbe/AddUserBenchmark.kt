package ch.unine.anonymbe

import ch.unine.anonymbe.api.User
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
open class AddUserBenchmark : AdminBenchmark() {
    @Benchmark
    @BenchmarkMode(Mode.Throughput, Mode.SampleTime)
    fun addUserBenchmark(ts: CounterThreadState) {
        val name = "testuser${++ts.counter}"
        val user = User(name)
        try {
            service.createUser(user).execute()
        } catch (_: Exception) {
            errors++
        }
    }
}
