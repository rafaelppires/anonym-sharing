package ch.unine.anonymbe.storage

import software.amazon.awssdk.auth.credentials.AwsCredentials
import software.amazon.awssdk.core.sync.RequestBody
import software.amazon.awssdk.http.SdkHttpConfigurationOption
import software.amazon.awssdk.http.apache.ApacheHttpClient
import software.amazon.awssdk.regions.Region
import software.amazon.awssdk.services.s3.S3Client
import software.amazon.awssdk.services.s3.model.BucketAlreadyOwnedByYouException
import software.amazon.awssdk.services.s3.model.NoSuchBucketException
import software.amazon.awssdk.utils.AttributeMap
import java.io.InputStream
import java.net.URI

class Aws(
    endpointUrl: String = DEFAULT_ENDPOINT,
    accessKey: String = DEFAULT_ACCESS_KEY,
    secretKey: String = DEFAULT_SECRET_KEY
) : StorageApi {
    private val httpClient = ApacheHttpClient.builder()
        .buildWithDefaults(
            AttributeMap.builder()
                .put(SdkHttpConfigurationOption.TRUST_ALL_CERTIFICATES, true).build()
        )
    private val client: S3Client = S3Client.builder()
        .endpointOverride(URI.create(endpointUrl))
        .region(Region.US_EAST_1)
        .serviceConfiguration { it.pathStyleAccessEnabled(true) }
        .httpClient(httpClient)
        .credentialsProvider {
            object : AwsCredentials {
                override fun accessKeyId(): String = accessKey

                override fun secretAccessKey(): String = secretKey
            }
        }
        .build()

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
        data: InputStream,
        dataLength: Long,
        mime: String
    ) {
        client.putObject(
            {
                it.bucket(bucketName)
                it.contentType(mime)
                it.key(objectName)
            },
            RequestBody.fromInputStream(data, dataLength)
        )
    }

    override fun getObject(bucketName: String, objectName: String): InputStream = client.getObject {
        it.bucket(bucketName)
        it.key(objectName)
    }

    companion object {
        const val DEFAULT_ENDPOINT = "https://hoernli-5.maas:30900"
        const val DEFAULT_ACCESS_KEY = "access"
        const val DEFAULT_SECRET_KEY = "secretkey"
    }
}
