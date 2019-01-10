package ch.unine.anonymbe

import ch.unine.anonymbe.api.AdminApi
import ch.unine.anonymbe.api.Api
import ch.unine.anonymbe.api.User
import java.util.*

fun main(args: Array<String>) {
    val service = Api.build<AdminApi>()

    service.deleteUser(User("toto")).execute()
    val call = service.createUser(User("toto"))
    val result = call.execute()
    println("HTTP result = ${result.isSuccessful}")
    result.body()?.let { body ->
        println("JSON result = ${body.result}")
        println("userKey = ${body.userKey}")
        println("userKey(bin) = ${String(Base64.getDecoder().decode(body.userKey))}")
    }
}
