package ch.unine.anonymbe.storage

import ch.unine.anonymbe.api.Api
import ch.unine.anonymbe.api.WriterProxyApi
import ch.unine.anonymbe.api.throwExceptionIfNotSuccessful
import okhttp3.MediaType
import okhttp3.RequestBody
import java.io.InputStream

class WriterProxy(
    writerProxyUrl: String = DEFAULT_URL,
    minioUrl: String = Minio.DEFAULT_ENDPOINT,
    accessKey: String = Minio.DEFAULT_ACCESS_KEY,
    secretKey: String = Minio.DEFAULT_SECRET_KEY
) :
    Minio(minioUrl, accessKey, secretKey) {
    private val api: WriterProxyApi = Api.build(writerProxyUrl)

    override fun createBucketIfNotExists(bucketName: String) {
        // Nothing
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

        api.uploadFile(bucketName, objectName, requestBody).execute().throwExceptionIfNotSuccessful()
    }

    companion object {
        const val DEFAULT_URL = "https://hoernli-5.maas:30555"
    }
}
