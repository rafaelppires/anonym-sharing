package ch.unine.anonymbe.storage

import ch.unine.anonymbe.api.Api
import ch.unine.anonymbe.api.WriterProxyApi
import ch.unine.anonymbe.api.throwExceptionIfNotSuccessful
import okhttp3.MediaType
import okhttp3.RequestBody
import java.io.InputStream

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
        data: InputStream,
        dataLength: Long,
        mime: String
    ) {
        val dataBytes = data.readNBytes(dataLength.toInt())
        val requestBody = RequestBody.create(MediaType.get(mime), dataBytes)

        writerProxy.uploadFile(bucketName, objectName, requestBody).execute().throwExceptionIfNotSuccessful()
    }

    companion object {
        const val DEFAULT_URL = "https://hoernli-4.maas:30555"
    }
}
