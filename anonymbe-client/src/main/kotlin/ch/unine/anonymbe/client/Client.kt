package ch.unine.anonymbe.client

import ch.unine.anonymbe.api.*
import ch.unine.anonymbe.storage.StorageApi
import retrofit2.Call
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

    fun generateSymmetricKeyAndGetEnvelope(groupId: String): Pair<SymmetricKey, String> {
        val key = generateKey()
        val bucket = Bucket(userId, groupId, b64Encoder.encodeToString(key.encoded))
        val envelopeCall = apiClient.getEnvelope(bucket).execute()
        envelopeCall.throwExceptionIfNotReallySuccessful()

        val envelope = envelopeCall.body()?.ciphertext!!

        return Pair(key, envelope)
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
         *  16 bytes of AES-GCM tag
         */
        val userKeySpec = SecretKeySpec(userKey, KEY_ALGORITHM)
        val envelopeBuffer = ByteBuffer.wrap(envelope)
        val encryptedKey = ByteArray(16 + 32 + 16)
        var decryptedKey: ByteArray? = null

        while (envelopeBuffer.hasRemaining()) {
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
            val paramSpec = GCMParameterSpec(CIPHER_TAG_BYTES * 8, iv)
            cipher.init(opMode, key, paramSpec)
        }

    @Throws(AEADBadTagException::class)
    private fun cipherAes(data: ByteArray, key: SymmetricKey, associatedData: ByteArray?, opMode: Int): ByteArray {
        val dataBuffer = ByteBuffer.wrap(data)

        val iv = ByteArray(CIPHER_IV_BYTES).also {
            when (opMode) {
                Cipher.ENCRYPT_MODE -> random.nextBytes(it)
                Cipher.DECRYPT_MODE -> dataBuffer.get(it)
                else -> throw IllegalArgumentException()
            }
        }
        val cipher = initCipher(key, iv, opMode)
        associatedData?.let {
            cipher.updateAAD(it)
        }

        val processedData = cipher.doFinal(data, CIPHER_IV_BYTES, data.size - CIPHER_IV_BYTES)
        return when (opMode) {
            Cipher.ENCRYPT_MODE -> {
                val result = ByteBuffer.allocate(CIPHER_IV_BYTES + processedData.size)
                result.put(iv)
                result.put(processedData)
                return result.array()
            }
            Cipher.DECRYPT_MODE -> processedData
            else -> throw IllegalArgumentException()
        }
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

    private fun generateKey(): SymmetricKey = ByteArray(CIPHER_KEY_BYTES).let {
        random.nextBytes(it)
        SecretKeySpec(it, KEY_ALGORITHM)
    }

    fun verifyEnvelope(envelope: String, userKey: String) {
        tryDecrypt(ByteArray(0), b64Decoder.decode(envelope), b64Decoder.decode(userKey))
    }

    companion object {
        private const val CIPHER_ALGORITHM = "AES/GCM/NoPadding"
        private const val KEY_ALGORITHM = "AES"
        private const val CIPHER_KEY_BYTES = 32
        private const val CIPHER_TAG_BYTES = 16
        private const val CIPHER_IV_BYTES = 16
    }
}
