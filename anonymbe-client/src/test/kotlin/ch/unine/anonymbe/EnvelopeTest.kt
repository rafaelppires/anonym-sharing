package ch.unine.anonymbe

import ch.unine.anonymbe.client.Envelope
import ch.unine.anonymbe.client.SymmetricKey
import java.util.*
import kotlin.test.Test

class EnvelopeTest {
    @Test
    fun testEnvelope1() {
        val b64Decoder = Base64.getDecoder()

        val envelopeData = b64Decoder.decode("Ak0neqWxmZ9tJjhZql5mkNfq5HIt35uxxB9/dEsv6KJ4bgVa5QhLa/WFOXKRR3ihQvtDdkLz80DYPTPmfzZdFKB6EtAnHK0kRSfBrdsYvaURHF7nacTD/2JyYhQk3yOf64i0CXuKXGYP0anUhSfEKExRs5H9Wz+r")
        val envelope = Envelope(envelopeData)
        val userKeyData = b64Decoder.decode("OYJhvTD9aSUW8MFwlpQ07Yf8cEsOIzKCKbOmWaG6S54=")
        val userKey = SymmetricKey(userKeyData)

        // If wrong, exception will be thrown
        println(envelope.open(userKey).toString(Charsets.UTF_8))
    }
}
