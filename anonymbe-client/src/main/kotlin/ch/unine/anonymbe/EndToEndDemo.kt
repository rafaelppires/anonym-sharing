package ch.unine.anonymbe

import ch.unine.anonymbe.api.*
import ch.unine.anonymbe.client.Client
import ch.unine.anonymbe.storage.Minio
import kotlinx.coroutines.runBlocking
import java.net.ProtocolException
import java.util.*
import kotlin.random.Random

fun main(args: Array<String>) = runBlocking<Unit> {
    val userId = UUID.randomUUID().toString()
    val groupId = "testgroupdemo"
    val filename = "testFileDemo"

    val storageClient = Minio()

    val adminApi = Api.build(AdminApi::class)

    println("Creating user")
    val userResult = adminApi.createUser(User(userId)).await()
    val userKey = if (userResult.isSuccessful) {
        userResult.body()?.user_key ?: throw Exception("Cannot get user key")
    } else {
        throw Exception("Cannot create user: ${userResult.errorBody()?.string()}")
    }

    println("Creating group")
    try {
        val groupResult = adminApi.createGroup(UserGroup(userId, groupId)).await()
        if (!groupResult.isSuccessful) {
            println("Cannot create group: ${userResult.errorBody()?.string()}")
        }
    } catch (e: ProtocolException) {
        e.printStackTrace()
    }

    println("Admin part done")

    val client = Client(userId, Api.DEFAULT_URL, storageClient)

    println("Generating symmetric key and asking refmon for envelope")
    val (symmetricKey, deferredEnvelope) = client.generateSymmetricKeyAndGetEnvelope(groupId)

    println("Generating dummy data")
    val dummyData = Random.nextBytes(4 * 1024)

    println("Await on envelope call")
    val envelopeResult = deferredEnvelope.await()
    envelopeResult.throwExceptionIfNotReallySuccessful()

    println("Success on getting the envelope")

    println("Opening the envelope")
    val envelope = envelopeResult.body()!!.ciphertext

    println("Upload encrypted file and envelope to the cloud")
    client.uploadToCloud(dummyData, envelope, symmetricKey, groupId, filename)

    println("Upload done")

    println("Retrieve data back")
    val retrievedData: ByteArray = client.retrieveFromCloud(userKey, groupId, filename)

    val b64Encoder = Base64.getEncoder()
    println("Original data: ${b64Encoder.encode(dummyData)}")
    println("Retrieved data: ${b64Encoder.encode(retrievedData)}")
}
