package ch.unine.anonymbe.client

import ch.unine.anonymbe.api.Api
import ch.unine.anonymbe.api.Bucket
import ch.unine.anonymbe.api.EnvelopeResult
import ch.unine.anonymbe.api.UserApi
import ch.unine.anonymbe.storage.StorageApi
import kotlinx.coroutines.Deferred
import retrofit2.Response
import java.io.*
import java.nio.ByteBuffer
import java.security.SecureRandom
import java.util.*
import javax.crypto.AEADBadTagException
import javax.crypto.Cipher
import javax.crypto.spec.GCMParameterSpec
import javax.crypto.spec.SecretKeySpec

typealias SymmetricKey = SecretKeySpec

class Client(private val userId: String, apiUrl: String, private val storageClient: StorageApi) {
    private val apiClient: UserApi = Api.build(UserApi::class, apiUrl)
    private val b64Encoder: Base64.Encoder = Base64.getEncoder()
    private val b64Decoder: Base64.Decoder = Base64.getDecoder()
    private val random = SecureRandom()

    fun generateSymmetricKeyAndGetEnvelope(groupId: String): Pair<SymmetricKey, Deferred<Response<EnvelopeResult>>> {
        val key = generateKey()
        val bucket = Bucket(userId, groupId, b64Encoder.encodeToString(key.encoded))
        val envelopeDeferred = apiClient.getEnvelope(bucket)

        return Pair(key, envelopeDeferred)
    }

    fun uploadToCloud(
        data: ByteArray,
        envelope: String,
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

        val envelope = inputStream.readNBytes(envelopeLength)
        val encryptedData = inputStream.readAllBytes()
        inputStream.close()

        return tryDecrypt(encryptedData, envelope, b64Decoder.decode(userKey))
    }

    private fun tryDecrypt(encryptedData: ByteArray, envelope: ByteArray, userKey: ByteArray): ByteArray {
        /*
         * Format of the envelope:
         *  16 bytes of IV
         *  32 bytes of encrypted key
         */
        val userKeySpec = SecretKeySpec(userKey, KEY_ALGORITHM)
        val envelopeBuffer = ByteBuffer.wrap(envelope)
        val iv = ByteArray(16)
        val encryptedKey = ByteArray(32)
        var decryptedKey: ByteArray? = null

        while (envelopeBuffer.hasRemaining()) {
            envelopeBuffer.get(iv)
            envelopeBuffer.get(encryptedKey)

            try {
                decryptedKey = decryptAes(encryptedKey, userKeySpec)
                break
            } catch (e: AEADBadTagException) {
                continue
            }
        }

        if (decryptedKey == null) {
            throw Exception("User key not in envelope")
        }

        return decryptAes(encryptedData, SecretKeySpec(decryptedKey, KEY_ALGORITHM), envelope)
    }

    private fun encryptAndEncodeFile(data: ByteArray, envelope: String, key: SymmetricKey): Pair<InputStream, Long> {
        val binaryEnvelope = b64Decoder.decode(envelope)
        val encryptedData = encryptAes(data, key, binaryEnvelope)

        val (envelopeAndData, length) = encodeEnvelopeAndData(binaryEnvelope, encryptedData)
        return Pair(envelopeAndData, length)
    }

    private fun initCipher(key: SymmetricKey, iv: ByteArray, opMode: Int) =
        Cipher.getInstance(CIPHER_ALGORITHM).also { cipher ->
            val paramSpec = GCMParameterSpec(CIPHER_KEY_BITS, iv)
            cipher.init(opMode, key, paramSpec)
        }

    @Throws(AEADBadTagException::class)
    private fun cipherAes(data: ByteArray, key: SymmetricKey, associatedData: ByteArray?, opMode: Int): ByteArray {
        val dataBuffer = ByteBuffer.wrap(data)

        val iv = ByteArray(CIPHER_IV_BITS / 8).also {
            when (opMode) {
                Cipher.ENCRYPT_MODE -> random.nextBytes(it)
                Cipher.DECRYPT_MODE -> dataBuffer.get(it)
            }
        }
        val cipher = initCipher(key, iv, opMode)
        associatedData?.let {
            cipher.updateAAD(it)
        }
        val processedData = cipher.doFinal(data)
        val result = ByteBuffer.allocate((CIPHER_IV_BITS / 8) + processedData.size)
        result.put(iv)
        result.put(processedData)
        return result.array()
    }

    @Throws(AEADBadTagException::class)
    private fun encryptAes(data: ByteArray, key: SymmetricKey, associatedData: ByteArray? = null) =
        cipherAes(data, key, associatedData, Cipher.ENCRYPT_MODE)

    @Throws(AEADBadTagException::class)
    private fun decryptAes(data: ByteArray, key: SymmetricKey, associatedData: ByteArray? = null) =
        cipherAes(data, key, associatedData, Cipher.DECRYPT_MODE)

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

    private fun generateKey(): SymmetricKey = ByteArray(CIPHER_KEY_BITS / 8).let {
        random.nextBytes(it)
        SecretKeySpec(it, KEY_ALGORITHM)
    }

    companion object {
        private const val CIPHER_ALGORITHM = "AES/GCM/NoPadding"
        private const val KEY_ALGORITHM = "AES"
        private const val CIPHER_KEY_BITS = 128
        private const val CIPHER_IV_BITS = 128
    }
}
