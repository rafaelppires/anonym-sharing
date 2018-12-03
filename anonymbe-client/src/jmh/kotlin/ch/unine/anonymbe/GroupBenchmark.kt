package ch.unine.anonymbe

import ch.unine.anonymbe.api.User
import ch.unine.anonymbe.api.UserGroup
import org.openjdk.jmh.annotations.*
import org.openjdk.jmh.infra.ThreadParams

@State(Scope.Thread)
open class GroupThreadState {
    private var counter = 0
    private var max = -1

    fun next(): Int {
        if (counter >= max) {
            throw IllegalStateException("Increase number of users")
        }

        return counter++
    }

    @Setup(Level.Iteration)
    fun setup(params: ThreadParams) {
        val slice = GroupBenchmark.USERS_AMOUNT / params.threadCount
        counter = params.threadIndex * slice
        max = ((params.threadIndex + 1) * slice) - 1
    }
}

@State(Scope.Benchmark)
open class GroupBenchmark : AdminBenchmark() {
    @Setup(Level.Iteration)
    fun fillDatabase() {
        println("Filling database")
        val users = (1..USERS_AMOUNT).map { "user$it" }
        users.parallelStream().forEach {
            service.createUser(User(it)).execute()
        }
        println("Filled")
    }

    @Benchmark
    @BenchmarkMode(Mode.Throughput, Mode.SampleTime)
    fun addUserToGroupBenchmark(ts: GroupThreadState) {
        val name = "user${ts.next()}"
        val userGroup = UserGroup(name, "group")
        try {
            service.addUserToGroup(userGroup).execute()
        } catch (_: Exception) {
            errors++
        }
    }

    companion object {
        const val USERS_AMOUNT = 10_000
    }
}
