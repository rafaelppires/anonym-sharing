package ch.unine.anonymbe.client

import java.nio.ByteBuffer
import java.security.SecureRandom
import javax.crypto.AEADBadTagException
import javax.crypto.Cipher
import javax.crypto.spec.GCMParameterSpec
import javax.crypto.spec.SecretKeySpec

typealias SymmetricKey = SecretKeySpec

@Suppress("FunctionName")
fun SymmetricKey(key: ByteArray) = SymmetricKey(key, Encryption.KEY_ALGORITHM)

const val CIPHER_KEY_BYTES = 32
const val CIPHER_TAG_BYTES = 16
const val CIPHER_IV_BYTES = 12

object Encryption {
    private val random by lazy { SecureRandom() }

    private fun initCipher(key: SymmetricKey, iv: ByteArray, opMode: Int) =
        Cipher.getInstance(CIPHER_ALGORITHM).also { cipher ->
            val paramSpec = GCMParameterSpec(CIPHER_TAG_BYTES * 8, iv)
            cipher.init(opMode, key, paramSpec)
        }

    fun generateKey(): SymmetricKey = ByteArray(CIPHER_KEY_BYTES).let {
        random.nextBytes(it)
        SymmetricKey(it)
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

        val dataOffset = when (opMode) {
            Cipher.DECRYPT_MODE -> CIPHER_IV_BYTES
            else -> 0
        }
        val processedData = cipher.doFinal(data, dataOffset, data.size - dataOffset)
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

    fun encryptAes(data: ByteArray, key: SymmetricKey, associatedData: ByteArray? = null) =
        cipherAes(data, key, associatedData, Cipher.ENCRYPT_MODE)

    @Throws(AEADBadTagException::class)
    fun decryptAes(data: ByteArray, key: SymmetricKey, associatedData: ByteArray? = null) =
        cipherAes(data, key, associatedData, Cipher.DECRYPT_MODE)

    fun encryptAes(data: ByteArray, key: ByteArray, associatedData: ByteArray? = null) =
        cipherAes(data, SymmetricKey(key, KEY_ALGORITHM), associatedData, Cipher.ENCRYPT_MODE)

    @Throws(AEADBadTagException::class)
    fun decryptAes(data: ByteArray, key: ByteArray, associatedData: ByteArray? = null) =
        cipherAes(data, SymmetricKey(key, KEY_ALGORITHM), associatedData, Cipher.DECRYPT_MODE)

    private const val CIPHER_ALGORITHM = "AES/GCM/NoPadding"
    internal const val KEY_ALGORITHM = "AES"
}
