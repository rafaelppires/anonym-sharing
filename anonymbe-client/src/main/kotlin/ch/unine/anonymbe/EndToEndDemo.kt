package ch.unine.anonymbe

import ch.unine.anonymbe.api.AdminApi
import ch.unine.anonymbe.api.Api
import ch.unine.anonymbe.api.User
import ch.unine.anonymbe.api.UserGroup
import ch.unine.anonymbe.client.Client
import ch.unine.anonymbe.client.IndexedEnvelope
import ch.unine.anonymbe.storage.Minio
import java.util.*
import kotlin.random.Random

fun main() {
    endToEndDemo(Random.nextBytes(512))
}

const val groupId = "endtoend"
val storageClient = Minio()

typealias EnvelopeClass = IndexedEnvelope

/**
 * Do an end-to-end demonstration of the full system.
 * @param dummyData Data to send to the Cloud
 * @return If all goes well, the same dummyData after it has done a round trip to the Cloud storage.
 */
fun endToEndDemo(dummyData: ByteArray): ByteArray = try {
    val userId = UUID.randomUUID().toString()
    val filename = "testFileDemo"
    println("userId = $userId, groupId = $groupId, filename = $filename")

    val adminApi = Api.build<AdminApi>()

    println("Creating user")
    val userResult = adminApi.createUser(User(userId)).execute()
    val userKey = if (userResult.isSuccessful) {
        userResult.body()?.userKey ?: throw Exception("Cannot get user key")
    } else {
        throw Exception("Cannot create user: ${userResult.errorBody()?.string()}")
    }

    println("Creating group")
    val groupResult = adminApi.createGroup(UserGroup(userId, groupId)).execute()
    if (!groupResult.isSuccessful) {
        println("Cannot create group: ${userResult.errorBody()?.string()}")
    }

    println("Admin part done")

    val client = Client(userId, userKey, Api.DEFAULT_URL, storageClient) { EnvelopeClass(it) }

    println("Generating symmetric key and asking refmon for envelope")
    val (symmetricKey, envelope) = client.generateSymmetricKeyAndGetEnvelope(groupId)

    println("Verifying envelope")
    if (client.verifyEnvelope(envelope, symmetricKey.encoded)) {
        println("Success on getting the envelope")
    } else {
        throw Exception("Content of the envelope is not what we sent!")
    }

    println("Upload encrypted file and envelope to the cloud")
    client.uploadToCloud(dummyData, envelope, symmetricKey, groupId, filename)

    println("Upload done")

    println("Retrieve data back")
    val retrievedData: ByteArray = client.retrieveFromCloud(groupId, filename)

    val b64Encoder = Base64.getEncoder()
    println("Original data: ${b64Encoder.encodeToString(dummyData)}")
    println("Retrieved data: ${b64Encoder.encodeToString(retrievedData)}")

    retrievedData
} finally {
    try {
        storageClient.deleteBucket(groupId)
    } catch (e: Exception) {
        e.printStackTrace()
    }
}
