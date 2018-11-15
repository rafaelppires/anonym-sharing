package ch.unine.anonymbe

import ch.unine.anonymbe.api.AdminApi
import ch.unine.anonymbe.api.Api
import ch.unine.anonymbe.api.User
import ch.unine.anonymbe.api.UserGroup
import ch.unine.anonymbe.client.Client
import ch.unine.anonymbe.storage.Minio
import java.util.*
import kotlin.random.Random

fun main(args: Array<String>) {
    val userId = UUID.randomUUID().toString()
    val groupId = UUID.randomUUID().toString()
    val filename = "testFileDemo"

    val storageClient = Minio()

    val adminApi = Api.build(AdminApi::class)

    println("Creating user")
    val userResult = adminApi.createUser(User(userId)).execute()
    val userKey = if (userResult.isSuccessful) {
        userResult.body()?.user_key ?: throw Exception("Cannot get user key")
    } else {
        throw Exception("Cannot create user: ${userResult.errorBody()?.string()}")
    }

    println("Creating group")
    val groupResult = adminApi.createGroup(UserGroup(userId, groupId)).execute()
    if (!groupResult.isSuccessful) {
        println("Cannot create group: ${userResult.errorBody()?.string()}")
    }

    println("Admin part done")

    val client = Client(userId, Api.DEFAULT_URL, storageClient)

    println("Generating dummy data")
    val dummyData = Random.nextBytes(512)

    println("Generating symmetric key and asking refmon for envelope")
    val (symmetricKey, envelope) = client.generateSymmetricKeyAndGetEnvelope(groupId)

    println("Verifying envelope")
    if (client.verifyEnvelope(userKey, envelope, symmetricKey.encoded)) {
        println("Success on getting the envelope")
    } else {
        throw Exception("Content of the envelope is not what we sent!")
    }

    println("Upload encrypted file and envelope to the cloud")
    client.uploadToCloud(dummyData, envelope, symmetricKey, groupId, filename)

    println("Upload done")

    println("Retrieve data back")
    val retrievedData: ByteArray = client.retrieveFromCloud(userKey, groupId, filename)

    val b64Encoder = Base64.getEncoder()
    println("Original data: ${b64Encoder.encodeToString(dummyData)}")
    println("Retrieved data: ${b64Encoder.encodeToString(retrievedData)}")
}
