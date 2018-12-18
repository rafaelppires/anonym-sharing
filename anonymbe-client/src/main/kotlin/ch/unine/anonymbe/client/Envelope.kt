package ch.unine.anonymbe.client

import ch.unine.anonymbe.api.EnvelopeResult
import java.nio.ByteBuffer
import java.security.MessageDigest
import java.util.*
import javax.crypto.AEADBadTagException

open class Envelope(protected val envelope: ByteArray) {
    constructor(envelopeBase64: String) : this(base64Decoder.decode(envelopeBase64))

    constructor(envelopeResult: EnvelopeResult) : this(envelopeResult.ciphertext)

    /**
     * ByteArray that backs this envelope. Do not modify contents!
     */
    internal val unsafeRaw get() = envelope

    /**
     * ByteArray that backs this envelope. Safe to use, returns a copy.
     */
    val raw = envelope.copyOf()

    @Throws(Exception::class)
    open fun open(userKey: SymmetricKey): ByteArray {
        val envelopeBuffer = ByteBuffer.wrap(envelope)
        val encryptedKey = ByteArray(CIPHER_IV_BYTES + CIPHER_KEY_BYTES + CIPHER_TAG_BYTES)
        var decryptedKey: ByteArray? = null

        while (envelopeBuffer.hasRemaining()) {
            envelopeBuffer.get(encryptedKey)

            try {
                decryptedKey = Encryption.decryptAes(encryptedKey, userKey)
                break
            } catch (e: AEADBadTagException) {
                continue
            }
        }

        if (decryptedKey == null) {
            throw Exception("User key not in envelope")
        }
        return decryptedKey
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
        @JvmStatic
        protected val base64Decoder: Base64.Decoder by lazy { Base64.getDecoder() }
    }
}

class IndexedEnvelope(envelope: ByteArray) : Envelope(envelope) {
    constructor(envelopeBase64: String) : this(base64Decoder.decode(envelopeBase64))

    constructor(envelopeResult: EnvelopeResult) : this(envelopeResult.ciphertext)

    override fun open(userKey: SymmetricKey): ByteArray {
        val envelopeBuffer: ByteBuffer = ByteBuffer.wrap(envelope)
        val encryptedKey = ByteArray(CIPHER_IV_BYTES + CIPHER_KEY_BYTES + CIPHER_TAG_BYTES)
        val hashedIndex = ByteArray(HASHED_INDEX_BYTES)
        var decryptedKey: ByteArray? = null

        val hashToFind: ByteArray = digester.digest(ByteArray(NONCE_BYTES + CIPHER_KEY_BYTES).let {
            envelopeBuffer.get(it, 0, NONCE_BYTES)
            userKey.encoded.copyInto(it, NONCE_BYTES)
        })

        while (envelopeBuffer.hasRemaining()) {
            envelopeBuffer.get(hashedIndex)

            if (hashedIndex contentEquals hashToFind) {
                envelopeBuffer.get(encryptedKey)
                decryptedKey = Encryption.decryptAes(encryptedKey, userKey)
                break
            } else {
                envelopeBuffer.position(envelopeBuffer.position() + encryptedKey.size)
            }
        }

        if (decryptedKey == null) {
            throw Exception("User key not in envelope")
        }
        return decryptedKey
    }

    companion object {
        private const val NONCE_BYTES = 12
        private const val HASHED_INDEX_BYTES = 28

        private val digester: MessageDigest = MessageDigest.getInstance("SHA-224")
    }
}
