package ch.unine.anonymbe.client

import ch.unine.anonymbe.api.EnvelopeResult
import java.nio.ByteBuffer
import java.util.*
import javax.crypto.AEADBadTagException

data class Envelope(private val envelope: ByteArray) {
    constructor(envelopeBase64: String) : this(base64Decoder.decode(envelopeBase64))

    constructor(envelopeResult: EnvelopeResult) : this(envelopeResult.ciphertext)

    /**
     * ByteArray that backs this envelope. Do not modify contents!
     */
    internal val unsafeRaw = envelope

    /**
     * ByteArray that backs this envelope. Safe to use, returns a copy.
     */
    val raw = envelope.copyOf()

    @Throws(Exception::class)
    fun open(userKey: SymmetricKey): ByteArray {
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

    companion object {
        private val base64Decoder by lazy { Base64.getDecoder() }
    }
}
