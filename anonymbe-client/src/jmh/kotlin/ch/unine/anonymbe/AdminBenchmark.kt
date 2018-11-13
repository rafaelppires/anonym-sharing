package ch.unine.anonymbe

import ch.unine.anonymbe.api.AdminApi
import ch.unine.anonymbe.api.Api
import ch.unine.anonymbe.api.User
import kotlinx.coroutines.runBlocking
import org.openjdk.jmh.annotations.Benchmark
import org.openjdk.jmh.annotations.Scope
import org.openjdk.jmh.annotations.State

@State(Scope.Thread)
open class AdminBenchmark {
    private val service = Api.build(AdminApi::class)
    private var counter = 0

    @Benchmark
    fun addUserBenchmark() = runBlocking<Unit> {
        val name = "testuser${++counter}"
        val user = User(name)
        service.createUser(user).await()
    }
}
