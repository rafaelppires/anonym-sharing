package ch.unine.anonymbe

import ch.unine.anonymbe.storage.*
import org.junit.Assert
import java.util.*
import kotlin.test.Test

class AwsTest : StorageTest() {
    override val storageClient by lazy {
        RegularAws()
    }
}

class TokenAwsTest : StorageTest() {
    override val storageClient by lazy {
        TokenAws()
    }
}

class MinioTest : StorageTest() {
    override val storageClient by lazy {
        Minio()
    }
}

class WriterProxyAwsTest : StorageTest() {
    override val storageClient by lazy {
        WriterProxy(RegularAws())
    }
}

class WriterProxyRegularMinioTest : StorageTest() {
    override val storageClient by lazy {
        WriterProxy(Minio())
    }
}

class HybridTokenAwsMinioTest : StorageTest() {
    override val storageClient by lazy {
        HybridTokenAwsMinio()
    }
}

abstract class StorageTest {
    abstract val storageClient: StorageApi

    @Test
    fun storageTest() {
        val data: ByteArray =
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.".toByteArray()

        val filename = UUID.randomUUID().toString()
        println("UUID = $filename")

        val bucketName = "newbucket"
        storageClient.createBucketIfNotExists(bucketName)
        storageClient.storeObject(bucketName, filename, data)
        val retrievedData = storageClient.getObject(bucketName, filename).readAllBytes()
        Assert.assertArrayEquals(data, retrievedData)
    }
}
