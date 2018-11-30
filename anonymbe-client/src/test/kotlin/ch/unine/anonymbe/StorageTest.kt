package ch.unine.anonymbe

import ch.unine.anonymbe.storage.Aws
import ch.unine.anonymbe.storage.Minio
import ch.unine.anonymbe.storage.StorageApi
import ch.unine.anonymbe.storage.WriterProxy
import org.junit.Assert
import java.util.*
import kotlin.test.Test

class AwsTest : StorageTest() {
    override val storageClient by lazy {
        Aws()
    }
}

class MinioTest : StorageTest() {
    override val storageClient by lazy {
        Minio()
    }
}

class WriterProxyAwsTest : StorageTest() {
    override val storageClient by lazy {
        WriterProxy(Aws())
    }
}

class WriterProxyMinioTest : StorageTest() {
    override val storageClient by lazy {
        WriterProxy(Minio())
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

        storageClient.createBucketIfNotExists("bucket")
        storageClient.storeObject("bucket", filename, data.inputStream(), data.size.toLong())
        val retrievedData = storageClient.getObject("bucket", filename).readAllBytes()
        Assert.assertArrayEquals(data, retrievedData)
    }
}
