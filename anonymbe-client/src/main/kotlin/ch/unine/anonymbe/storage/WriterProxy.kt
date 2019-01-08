package ch.unine.anonymbe.storage

import ch.unine.anonymbe.api.Api
import ch.unine.anonymbe.api.WriterProxyApi
import ch.unine.anonymbe.api.throwExceptionIfNotSuccessful
import okhttp3.MediaType
import okhttp3.RequestBody

class WriterProxy(private val storageApi: StorageApi, writerProxyUrl: String = DEFAULT_URL) : StorageApi {
    private val writerProxy: WriterProxyApi = Api.build(writerProxyUrl)

    override fun deleteBucket(bucketName: String) = storageApi.deleteBucket(bucketName)

    override fun getObject(bucketName: String, objectName: String) = storageApi.getObject(bucketName, objectName)

    override fun createBucketIfNotExists(bucketName: String) {
        // Handled by writer proxy when calling storeObject
    }

    override fun storeObject(
        bucketName: String,
        objectName: String,
        data: ByteArray,
        mime: String
    ) {
        val requestBody = RequestBody.create(MediaType.get(mime), data)

        writerProxy.uploadFile(bucketName, objectName, requestBody).execute().throwExceptionIfNotSuccessful()
    }

    override fun deleteObject(bucketName: String, objectName: String) = storageApi.deleteObject(bucketName, objectName)

    companion object {
        const val DEFAULT_URL = "https://hoernli-4.maas:30555"
        const val DEFAULT_URL_TOKEN = "https://hoernli-4.maas:30556"
    }
}
