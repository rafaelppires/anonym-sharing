package ch.unine.anonymbe

import ch.unine.anonymbe.client.Encryption
import ch.unine.anonymbe.client.SymmetricKey
import org.junit.Assert
import org.junit.Test

class EncryptionTest {
    @Test
    fun testEncryptDecrypt() {
        val data = "Hello world!".repeat(40).toByteArray()
        val key = "12345678".repeat(4).toByteArray()
        val userKeySpec = SymmetricKey(key, "AES")

        val encrypted = Encryption.encryptAes(data, userKeySpec)
        val decrypted = Encryption.decryptAes(encrypted, userKeySpec)
        Assert.assertArrayEquals(data, decrypted)
    }
}
