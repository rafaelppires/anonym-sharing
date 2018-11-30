package ch.unine.anonymbe.storage

import java.io.InputStream

interface StorageApi {
    fun createBucketIfNotExists(bucketName: String)

    fun deleteBucket(bucketName: String)

    fun storeObject(
        bucketName: String, objectName: String,
        data: InputStream, dataLength: Long = data.available().toLong(),
        mime: String = "application/octet-stream"
    )

    fun getObject(bucketName: String, objectName: String): InputStream
}
