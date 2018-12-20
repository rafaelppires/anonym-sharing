package ch.unine.anonymbe.client

import java.nio.ByteBuffer
import java.security.MessageDigest
import java.util.*
import javax.crypto.AEADBadTagException

@ExperimentalUnsignedTypes
internal operator fun ByteArray.compareTo(other: ByteArray): Int {
    assert(this.size == other.size)

    for (i in 0 until size) {
        val comparison = this[i].toUByte().compareTo(other[i].toUByte())
        if (comparison != 0) {
            return comparison
        }
    }
    return 0
}

open class Envelope(protected val envelope: ByteArray) {
    /**
     * ByteArray that backs this envelope. Do not modify contents!
     */
    internal val unsafeRaw get() = envelope

    /**
     * ByteArray that backs this envelope. Safe to use, returns a copy.
     */
    val raw get() = envelope.copyOf()

    @Throws(Exception::class)
    open fun open(userKey: SymmetricKey): ByteArray {
        val envelopeBuffer = ByteBuffer.wrap(envelope)
        val encryptedKey = ByteArray(ENCRYPTED_KEY_BYTES)

        while (envelopeBuffer.hasRemaining()) {
            envelopeBuffer.get(encryptedKey)

            try {
                return Encryption.decryptAes(encryptedKey, userKey)
            } catch (e: AEADBadTagException) {
                continue
            }
        }
        throw Exception("User key not in envelope")
    }

    override fun equals(other: Any?): Boolean = when {
        this === other -> true
        javaClass != other?.javaClass -> false
        else -> envelope.contentEquals((other as Envelope).envelope)
    }

    override fun hashCode() = envelope.contentHashCode()

    override fun toString(): String {
        return "Envelope(raw=${Arrays.toString(envelope)})"
    }

    companion object {
        internal const val ENCRYPTED_KEY_BYTES = CIPHER_IV_BYTES + CIPHER_KEY_BYTES + CIPHER_TAG_BYTES
    }
}

class IndexedEnvelope(envelope: ByteArray) : Envelope(envelope) {
    @ExperimentalUnsignedTypes
    override fun open(userKey: SymmetricKey): ByteArray {
        val hashToFind: ByteArray = digester.digest(ByteArray(NONCE_BYTES + CIPHER_KEY_BYTES).also {
            envelope.copyInto(it, endIndex = NONCE_BYTES)
            userKey.encoded.copyInto(it, NONCE_BYTES)
        })

        // Get a ByteBuffer that starts where the keys start
        val envelopeBuffer: ByteBuffer = ByteBuffer.wrap(envelope).position(NONCE_BYTES).slice()

        // Buffer for loop
        val hashedIndex = ByteArray(HASHED_INDEX_BYTES)

        assert(envelopeBuffer.remaining() % INDEXED_ENCRYPTED_KEY_BYTES == 0)
        var left = 0
        var right = (envelopeBuffer.remaining() / INDEXED_ENCRYPTED_KEY_BYTES) - 1
        while (left <= right) {
            val currentIndex = left + (right - left) / 2
            envelopeBuffer.position(currentIndex * INDEXED_ENCRYPTED_KEY_BYTES)

            envelopeBuffer.get(hashedIndex)

            when {
                hashedIndex contentEquals hashToFind -> return Encryption.decryptAes(
                    ByteArray(ENCRYPTED_KEY_BYTES).also { envelopeBuffer.get(it) },
                    userKey
                )
                hashedIndex < hashToFind -> left++
                else -> right--
            }
        }

        throw Exception("User key not in envelope")
    }

    companion object {
        private const val NONCE_BYTES = 12
        private const val HASHED_INDEX_BYTES = 28
        private const val INDEXED_ENCRYPTED_KEY_BYTES = HASHED_INDEX_BYTES + ENCRYPTED_KEY_BYTES

        private val digester: MessageDigest = MessageDigest.getInstance("SHA-224")
    }
}
