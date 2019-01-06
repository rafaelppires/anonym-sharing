package ch.unine.anonymbe.storage

import software.amazon.awssdk.core.sync.RequestBody
import software.amazon.awssdk.http.SdkHttpClient
import software.amazon.awssdk.http.SdkHttpConfigurationOption
import software.amazon.awssdk.http.apache.ApacheHttpClient
import software.amazon.awssdk.services.s3.S3Client
import software.amazon.awssdk.services.s3.model.BucketAlreadyOwnedByYouException
import software.amazon.awssdk.services.s3.model.NoSuchBucketException
import software.amazon.awssdk.utils.AttributeMap
import java.io.InputStream

abstract class Aws : StorageApi {
    protected val httpClient: SdkHttpClient = ApacheHttpClient.builder()
        .buildWithDefaults(
            AttributeMap.builder()
                .put(SdkHttpConfigurationOption.TRUST_ALL_CERTIFICATES, true).build()
        )
    protected abstract val client: S3Client

    override fun createBucketIfNotExists(bucketName: String) {
        try {
            client.createBucket {
                it.bucket(bucketName)
            }
        } catch (_: BucketAlreadyOwnedByYouException) {
        }
    }

    override fun deleteBucket(bucketName: String) {
        try {
            client.listObjects {
                it.bucket(bucketName)
            }.contents().map {
                it.key()
            }.forEach { objectKey ->
                client.deleteObject {
                    it.bucket(bucketName)
                    it.key(objectKey)
                }
            }

            client.deleteBucket {
                it.bucket(bucketName)
            }
        } catch (_: NoSuchBucketException) {
        }
    }

    override fun storeObject(
        bucketName: String,
        objectName: String,
        data: ByteArray,
        mime: String
    ) {
        client.putObject(
            {
                it.bucket(bucketName)
                it.contentType(mime)
                it.key(objectName)
            },
            RequestBody.fromBytes(data)
        )
    }

    override fun getObject(bucketName: String, objectName: String): InputStream = client.getObject {
        it.bucket(bucketName)
        it.key(objectName)
    }

    override fun deleteObject(bucketName: String, objectName: String) {
        client.deleteObject {
            it.bucket(bucketName)
            it.key(objectName)
        }
    }

    companion object {
        const val DEFAULT_ENDPOINT = "https://hoernli-5.maas:30900"
    }
}
