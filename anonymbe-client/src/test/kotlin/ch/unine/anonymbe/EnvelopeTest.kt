package ch.unine.anonymbe

import ch.unine.anonymbe.client.Envelope
import ch.unine.anonymbe.client.IndexedEnvelope
import ch.unine.anonymbe.client.SymmetricKey
import org.junit.Assert
import java.util.*
import kotlin.test.Test

@ExperimentalUnsignedTypes
class EnvelopeTest {
    @Test
    fun testEnvelope1() {
        val b64Decoder = Base64.getDecoder()

        val envelopeData = b64Decoder.decode("Ak0neqWxmZ9tJjhZql5mkNfq5HIt35uxxB9/dEsv6KJ4bgVa5QhLa/WFOXKRR3ihQvtDdkLz80DYPTPmfzZdFKB6EtAnHK0kRSfBrdsYvaURHF7nacTD/2JyYhQk3yOf64i0CXuKXGYP0anUhSfEKExRs5H9Wz+r")
        val envelope = Envelope(envelopeData)
        val userKey: ByteArray = b64Decoder.decode("OYJhvTD9aSUW8MFwlpQ07Yf8cEsOIzKCKbOmWaG6S54=")

        // If wrong, exception will be thrown
        println(envelope.open(userKey).toString(Charsets.UTF_8))
    }

    @Test
    fun testIndexedEnvelope1() {
        val expected = "01234567890123456789012345678912"

        val b64Decoder = Base64.getDecoder()

        val envelopeData = b64Decoder.decode("g8zSlD/XL8T8uWadIQ36822ealzpuZhZLoiAy2SVt3dq4XnbWnTwPVHsSoxaFMUbI+FwKpsdjqB3Ta4yefBOoLgAFaCZkl2icvmP6OXR25MA/NqKSxwxRH7En8U9sZ/yyfrovf5xyJN1t63ud6QJe9Dh/ud5QOjsfPoESy8LjUF+vyuGc2/gQJb8HHR2zf2uo76SG1LBtC3sltp52OZs4SVjJIo/aCclEL+9IlbGvOLk3enVD6vYdylL24c=")
        val envelope = IndexedEnvelope(envelopeData)
        val userKey: ByteArray = b64Decoder.decode("4FF1S64GzoZuudEOj/hn4PSRYGX8HXZOq4eT9gqtKHg=")

        Assert.assertEquals(expected, envelope.open(userKey).toString(Charsets.UTF_8))
    }

    @Test
    fun testIndexedEnvelope2() {
        val expected = "01234567890123456789012345678912"

        val b64Decoder = Base64.getDecoder()

        val envelopeData = b64Decoder.decode("g8zSlD/XL8T8uWadIQ36822ealzpuZhZLoiAy2SVt3dq4XnbWnTwPVHsSoxaFMUbI+FwKpsdjqB3Ta4yefBOoLgAFaCZkl2icvmP6OXR25MA/NqKSxwxRH7En8U9sZ/yyfrovf5xyJN1t63ud6QJe9Dh/ud5QOjsfPoESy8LjUF+vyuGc2/gQJb8HHR2zf2uo76SG1LBtC3sltp52OZs4SVjJIo/aCclEL+9IlbGvOLk3enVD6vYdylL24c=")
        val envelope = IndexedEnvelope(envelopeData)
        val userKey: ByteArray = b64Decoder.decode("CIFMMADZOw2cSb6odpNISQAu4hLJIfK0NBkMn4vGhmQ=")

        Assert.assertEquals(expected, envelope.open(userKey).toString(Charsets.UTF_8))
    }
}
