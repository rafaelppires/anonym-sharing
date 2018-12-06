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

    private val queue: Queue<UserGroup> = ConcurrentLinkedQueue()

    @Setup(Level.Iteration)
    fun fillDatabase() {
        println("${queue.size} items left in queue")
        queue.clear()
        println("Filling database")
        val users = (1..(usersAmount.toInt())).map { "user$it" }
        users.parallelStream().forEach {
            service.createUser(User(it)).execute()
            queue.add(UserGroup(it, "creategrouptest"))
        }
        println("Filled")
    }

    @Benchmark
    @BenchmarkMode(Mode.Throughput)
    fun addUserToGroupBenchmark() {
        val userGroup = queue.remove()
        service.addUserToGroup(userGroup).execute().throwExceptionIfNotSuccessful()
    }
}
