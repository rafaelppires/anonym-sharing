package ch.unine.anonymbe

import ch.unine.anonymbe.client.Client
import ch.unine.anonymbe.client.SymmetricKey
import ch.unine.anonymbe.storage.Minio
import org.junit.Assert
import java.util.*
import kotlin.test.BeforeTest
import kotlin.test.Test

class EnvelopeTest {
    private lateinit var sut: Client

    @BeforeTest
    fun setupSut() {
        sut = Client("zz", "http://localhost", Minio())
    }

    @Test
    fun testEnvelope1() {
        val envelope = "Ak0neqWxmZ9tJjhZql5mkNfq5HIt35uxxB9/dEsv6KJ4bgVa5QhLa/WFOXKRR3ihQvtDdkLz80DYPTPmfzZdFKB6EtAnHK0kRSfBrdsYvaURHF7nacTD/2JyYhQk3yOf64i0CXuKXGYP0anUhSfEKExRs5H9Wz+r"
        val userKey = "OYJhvTD9aSUW8MFwlpQ07Yf8cEsOIzKCKbOmWaG6S54="

        val b64Decoder = Base64.getDecoder()

        println(sut.openEnvelope(userKey = b64Decoder.decode(userKey), envelope = b64Decoder.decode(envelope)).toString(Charsets.UTF_8))
    }

    @Test
    fun testEncryptDecrypt() {
        val data = "Hello world!".repeat(40).toByteArray()
        val key = "12345678".repeat(4).toByteArray()
        val userKeySpec = SymmetricKey(key, "AES")

        val encrypted = sut.encryptAes(data, userKeySpec)
        val decrypted = sut.decryptAes(encrypted, userKeySpec)
        Assert.assertArrayEquals(data, decrypted)
    }
}
