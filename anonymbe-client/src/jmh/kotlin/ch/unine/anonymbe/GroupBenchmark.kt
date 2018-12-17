package ch.unine.anonymbe

import ch.unine.anonymbe.api.User
import ch.unine.anonymbe.api.UserGroup
import ch.unine.anonymbe.api.throwExceptionIfNotSuccessful
import org.openjdk.jmh.annotations.*
import java.util.*
import java.util.concurrent.ConcurrentLinkedQueue

@State(Scope.Benchmark)
open class GroupBenchmark : AdminBenchmark() {
    @Param("20000")
    private var usersAmount: String = "0"

    @Param("false")
    private var preAddGroups: String = "false"

    private val queue: Queue<UserGroup> = ConcurrentLinkedQueue()

    @Setup(Level.Iteration)
    fun fillDatabase() {
        println("${queue.size} items left in queue")
        queue.clear()

        println("Filling database")
        val preAdd = preAddGroups.toBoolean()
        (1..(usersAmount.toInt()))
            .map { "user$it" }
            .parallelStream()
            .forEach { userId ->
                service.createUser(User(userId)).execute()
                val userGroup = UserGroup(userId, "creategrouptest")
                if (preAdd) {
                    service.addUserToGroup(userGroup).execute()
                }
                queue.add(userGroup)
            }
        println("Filled")
    }

    @Benchmark
    @BenchmarkMode(Mode.Throughput)
    fun addUserToGroupBenchmark() {
        val userGroup = queue.remove()
        service.addUserToGroup(userGroup).execute().throwExceptionIfNotSuccessful()
    }

    @Benchmark
    @BenchmarkMode(Mode.Throughput)
    fun deleteUserFromGroupBenchmark() {
        val userGroup = queue.remove()
        service.deleteUserFromGroup(userGroup).execute().throwExceptionIfNotSuccessful()
    }
}
