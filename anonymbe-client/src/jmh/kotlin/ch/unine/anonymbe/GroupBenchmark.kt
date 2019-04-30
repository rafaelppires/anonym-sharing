package ch.unine.anonymbe

import ch.unine.anonymbe.api.UserGroup
import org.openjdk.jmh.annotations.*
import java.util.concurrent.atomic.AtomicInteger

@State(Scope.Benchmark)
open class GroupBenchmark : AdminBenchmark() {
    @Param("20000")
    private var usersAmount: String = "0"

    @Param("0")
    private var usersPreadded: String = "0"

    private val userCounter: AtomicInteger = AtomicInteger(MAX_MAGNITUDE + 1)

    @Setup(Level.Iteration)
    fun fillDatabase() {
        println("Filling database")

        val process = ProcessBuilder(
            """docker run -ti --rm --network=host -v /home/ubuntu/mongobackup:/mongobackup:ro mongo:4.0.9-xenial
                | mongorestore --host=rs0/localhost:30000 -d newtest
                | --ssl --sslAllowInvalidCertificates --sslAllowInvalidHostnames
                | --gzip --archive=/mongobackup/envelope-state.mongo.gz""".trimMargin()
                .split(' ')
        )
            .redirectOutput(ProcessBuilder.Redirect.INHERIT)
            .redirectError(ProcessBuilder.Redirect.INHERIT)
            .start()

        if (process.waitFor() != 0) {
            throw Exception("Error running mongorestore")
        }

        println("Filled")
    }

    @Benchmark
    @BenchmarkMode(Mode.Throughput)
    fun addUserToGroupBenchmark() {
        val userId = userCounter.getAndIncrement()
        if (userId > (MAX_MAGNITUDE + ADDITIONAL_USERS)) {
            throw Exception("No more users to add")
        }
        val userGroup = UserGroup("$USER_NAME_PREFIX$userId", "$GROUP_NAME_PREFIX$usersPreadded")

        if (!service.addUserToGroup(userGroup).execute().isSuccessful) {
            errors++
        }
    }

    @Benchmark
    @BenchmarkMode(Mode.Throughput)
    fun deleteUserFromGroupBenchmark() {
        val userId = userCounter.getAndDecrement()
        if (userId < 1) {
            throw Exception("No more users to delete")
        }
        val userGroup = UserGroup("$USER_NAME_PREFIX$userId", "$GROUP_NAME_PREFIX$usersPreadded")

        if (!service.deleteUserFromGroup(userGroup).execute().isSuccessful) {
            errors++
        }
    }
}
