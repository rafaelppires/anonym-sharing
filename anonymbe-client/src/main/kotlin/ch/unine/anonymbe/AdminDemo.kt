package ch.unine.anonymbe

import ch.unine.anonymbe.api.AdminApi
import ch.unine.anonymbe.api.Api
import ch.unine.anonymbe.api.User
import kotlinx.coroutines.runBlocking

fun main(args: Array<String>) = runBlocking<Unit> {
    val service = Api.build(AdminApi::class)

    val call = service.createUser(User("toto"))
    val result = call.await()
    println("HTTP result = ${result.isSuccessful}")
    result.body()?.let { body ->
        println("JSON result = ${body.result}")
        println("user_key = ${body.user_key}")
    }
}
