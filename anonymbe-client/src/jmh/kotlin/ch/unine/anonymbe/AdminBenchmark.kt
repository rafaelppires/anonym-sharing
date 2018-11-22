package ch.unine.anonymbe

import ch.unine.anonymbe.api.AdminApi
import ch.unine.anonymbe.api.Api
import ch.unine.anonymbe.api.User
import ch.unine.anonymbe.api.throwExceptionIfNotReallySuccessful
import org.openjdk.jmh.annotations.*

@State(Scope.Thread)
open class AdminBenchmark {
    @Param("https://hoernli-6.maas:30444/", "https://hoernli-6.maas:30445/")
    @JvmField
    var endpointUrl: String = ""

    private lateinit var service: AdminApi
    private var counter = 0

    @Setup(Level.Trial)
    fun setup() {
        service = Api.build(endpointUrl)
        service.deleteAllData().execute().throwExceptionIfNotReallySuccessful()
    }

    @Benchmark
    fun addUserBenchmark() {
        val name = "testuser${++counter}"
        val user = User(name)
        service.createUser(user).execute()
    }
}
