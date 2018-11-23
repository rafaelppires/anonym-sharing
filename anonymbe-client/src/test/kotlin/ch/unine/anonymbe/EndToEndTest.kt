package ch.unine.anonymbe

import org.junit.Assert
import kotlin.test.Test

class EndToEndTest {
    @Test
    fun endToEndTest() {
        val data: ByteArray =
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.".toByteArray()

        Assert.assertArrayEquals(data, endToEndDemo(data))
    }
}
