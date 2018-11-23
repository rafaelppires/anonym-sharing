package ch.unine.anonymbe.client

import ch.unine.anonymbe.api.Api
import ch.unine.anonymbe.api.Bucket
import ch.unine.anonymbe.api.UserApi
import ch.unine.anonymbe.api.throwExceptionIfNotReallySuccessful
import ch.unine.anonymbe.storage.StorageApi
import java.io.*
import java.util.*

class Client(private val userId: String, apiUrl: String, private val storageClient: StorageApi) {
    private val apiClient: UserApi = Api.build(apiUrl)
    private val b64Encoder: Base64.Encoder = Base64.getEncoder()
    private val b64Decoder: Base64.Decoder = Base64.getDecoder()

    fun generateSymmetricKeyAndGetEnvelope(groupId: String): Pair<SymmetricKey, Envelope> {
        val key = Encryption.generateKey()
        val bucket = Bucket(userId, groupId, b64Encoder.encodeToString(key.encoded))
        val envelopeCall = apiClient.getEnvelope(bucket).execute()
        envelopeCall.throwExceptionIfNotReallySuccessful()

        val envelope = envelopeCall.body()?.let {
            Envelope(it)
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
        val (dataToStore, dataLength) = encryptAndEncodeFile(data, envelope, key)
        putObjectToStorage(dataToStore, dataLength, groupName, objectName)
    }

    fun retrieveFromCloud(userKey: String, groupId: String, filename: String): ByteArray {
        val inputStream = DataInputStream(storageClient.getObject(groupId, filename))
        val envelopeLength = inputStream.readInt()

        val envelope = Envelope(inputStream.readNBytes(envelopeLength))
        val encryptedData = inputStream.readAllBytes()
        inputStream.close()

        return tryDecrypt(encryptedData, envelope, SymmetricKey(b64Decoder.decode(userKey)))
    }

    private fun tryDecrypt(encryptedData: ByteArray, envelope: Envelope, userKey: SymmetricKey): ByteArray {
        /*
         * Format of the envelope:
         *  12 bytes of IV
         *  32 bytes of encrypted key
         *  16 bytes of AES-GCM tag
         */
        val decryptedKey: ByteArray = envelope.open(userKey)

        return Encryption.decryptAes(encryptedData, decryptedKey, envelope.unsafeRaw)
    }

    private fun encryptAndEncodeFile(data: ByteArray, envelope: Envelope, key: SymmetricKey): Pair<InputStream, Long> {
        val binaryEnvelope = envelope.unsafeRaw
        val encryptedData = Encryption.encryptAes(data, key, binaryEnvelope)

        val (envelopeAndData, length) = encodeEnvelopeAndData(binaryEnvelope, encryptedData)
        return Pair(envelopeAndData, length)
    }


    private fun encodeEnvelopeAndData(envelope: ByteArray, data: ByteArray): Pair<InputStream, Long> {
        val encodedEnvelope = ByteArrayOutputStream(Int.SIZE_BYTES + envelope.size)
        val envelopeDataOutputStream = DataOutputStream(encodedEnvelope)
        envelopeDataOutputStream.writeInt(envelope.size)
        envelopeDataOutputStream.write(envelope)

        val encodedEnvelopeInputStream = ByteArrayInputStream(encodedEnvelope.toByteArray())
        val length = Int.SIZE_BYTES.toLong() + envelope.size + data.size
        return Pair(SequenceInputStream(encodedEnvelopeInputStream, ByteArrayInputStream(data)), length)
    }

    private fun putObjectToStorage(
        encryptedData: InputStream,
        dataLength: Long,
        groupName: String,
        objectName: String
    ) {
        storageClient.storeObject(groupName, objectName, encryptedData, dataLength)
    }

    fun verifyEnvelope(userKey: String, envelope: Envelope, expectedContent: ByteArray): Boolean =
        expectedContent.contentEquals(envelope.open(SymmetricKey(b64Decoder.decode(userKey))))
}
