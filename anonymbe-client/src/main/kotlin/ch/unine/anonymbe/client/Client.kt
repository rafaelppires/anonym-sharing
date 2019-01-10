package ch.unine.anonymbe.client

import ch.unine.anonymbe.api.Api
import ch.unine.anonymbe.api.Bucket
import ch.unine.anonymbe.api.UserApi
import ch.unine.anonymbe.api.throwExceptionIfNotReallySuccessful
import ch.unine.anonymbe.storage.StorageApi
import java.io.ByteArrayOutputStream
import java.io.DataInputStream
import java.io.DataOutputStream
import java.util.*

class Client(
    private val userId: String,
    userKey: ByteArray,
    apiUrl: String,
    private val storageClient: StorageApi,
    private inline val envelopeProvider: (ByteArray) -> Envelope
) {
    private val apiClient: UserApi = Api.build(apiUrl)
    private val b64Encoder: Base64.Encoder = Base64.getEncoder()
    private val b64Decoder: Base64.Decoder = Base64.getDecoder()
    private val userKey = SymmetricKey(userKey)

    fun generateSymmetricKeyAndGetEnvelope(groupId: String): Pair<SymmetricKey, Envelope> {
        val key = Encryption.generateKey()
        val bucket = Bucket(userId, groupId, b64Encoder.encodeToString(key.encoded))
        val envelopeCall = apiClient.getEnvelope(bucket).execute()
        envelopeCall.throwExceptionIfNotReallySuccessful()

        val envelope = envelopeCall.body()?.let {
            envelopeProvider(b64Decoder.decode(it.ciphertext))
        } ?: throw NullPointerException("Envelope body is null")

        return Pair(key, envelope)
    }

    fun uploadToCloud(
        data: ByteArray,
        envelope: Envelope,
        key: SymmetricKey,
        groupName: String,
        objectName: String
    ) {
        storageClient.createBucketIfNotExists(groupName)
        val dataToStore = encryptAndEncodeFile(data, envelope, key)
        putObjectToStorage(dataToStore, groupName, objectName)
    }

    fun retrieveFromCloud(groupId: String, filename: String): ByteArray {
        val inputStream = DataInputStream(storageClient.getObject(groupId, filename))
        val envelopeLength = inputStream.readInt()

        val envelope = envelopeProvider(inputStream.readNBytes(envelopeLength))
        val encryptedData = inputStream.readAllBytes()
        inputStream.close()

        return tryDecrypt(encryptedData, envelope)
    }

    fun deleteFromCloud(groupId: String, filename: String) =
        storageClient.deleteObject(bucketName = groupId, objectName = filename)

    private fun tryDecrypt(encryptedData: ByteArray, envelope: Envelope): ByteArray {
        /*
         * Format of the envelope:
         *  12 bytes of IV
         *  32 bytes of encrypted key
         *  16 bytes of AES-GCM tag
         */
        val decryptedKey: ByteArray = envelope.open(userKey)

        return Encryption.decryptAes(encryptedData, decryptedKey, envelope.unsafeRaw)
    }

    private fun encryptAndEncodeFile(data: ByteArray, envelope: Envelope, key: SymmetricKey): ByteArray {
        val binaryEnvelope = envelope.unsafeRaw
        val encryptedData = Encryption.encryptAes(data, key, binaryEnvelope)

        return encodeEnvelopeAndData(binaryEnvelope, encryptedData)
    }


    private fun encodeEnvelopeAndData(envelope: ByteArray, data: ByteArray): ByteArray {
        val encodedEnvelope = ByteArrayOutputStream(Int.SIZE_BYTES + envelope.size + data.size)
        val envelopeDataOutputStream = DataOutputStream(encodedEnvelope)
        envelopeDataOutputStream.writeInt(envelope.size)
        envelopeDataOutputStream.write(envelope)
        envelopeDataOutputStream.write(data)
        envelopeDataOutputStream.flush()

        return encodedEnvelope.toByteArray()
    }

    private fun putObjectToStorage(
        encryptedData: ByteArray,
        groupName: String,
        objectName: String
    ) {
        storageClient.storeObject(groupName, objectName, encryptedData)
    }

    fun verifyEnvelope(envelope: Envelope, expectedContent: ByteArray): Boolean =
        expectedContent contentEquals envelope.open(userKey)
}
