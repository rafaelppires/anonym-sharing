package ch.unine.anonymbe

import ch.unine.anonymbe.api.User
import ch.unine.anonymbe.api.UserGroup
import org.openjdk.jmh.annotations.*
import java.util.*
import java.util.concurrent.ConcurrentLinkedQueue

@State(Scope.Benchmark)
open class GroupBenchmark : AdminBenchmark() {
    @Param("20000")
    private var usersAmount: String = "0"

    @Param("0")
    private var usersPreadded: String = "0"

    private val usersToAdd: Queue<UserGroup> = ConcurrentLinkedQueue()
    private val usersToRemove: Queue<UserGroup> = ConcurrentLinkedQueue()

    @Setup(Level.Iteration)
    fun fillDatabase() {
        println("${usersToAdd.size} items left in usersToAdd")
        println("${usersToRemove.size} items left in usersToRemove")
        usersToAdd.clear()

        println("Filling database")
        var preAdd = usersPreadded.toInt()
        val usersAmount = this.usersAmount.toInt() + preAdd

        service.createUser(User("dummyuser")).execute()
        service.createGroup(UserGroup("dummyuser", "creategrouptest")).execute()

        for (userId in (1..usersAmount).map { "user$it" }) {
            service.createUser(User(userId)).execute()
            val userGroup = UserGroup(userId, "creategrouptest")
            if ((preAdd--) > 0) {
                service.addUserToGroup(userGroup).execute()
                usersToRemove.add(userGroup)
            } else {
                usersToAdd.add(userGroup)
            }
        }
        println("Filled")
    }

    @Benchmark
    @BenchmarkMode(Mode.Throughput)
    fun addUserToGroupBenchmark() {
        val userGroup = usersToAdd.remove()
        if (!service.addUserToGroup(userGroup).execute().isSuccessful) {
            errors++
        }
    }

    @Benchmark
    @BenchmarkMode(Mode.Throughput)
    fun deleteUserFromGroupBenchmark() {
        val userGroup = usersToRemove.remove()
        if (!service.deleteUserFromGroup(userGroup).execute().isSuccessful) {
            errors++
        }
    }
}
