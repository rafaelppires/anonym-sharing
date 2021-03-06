package ch.unine.anonymbe.storage

import io.minio.MinioClient
import org.xmlpull.v1.XmlPullParserException
import java.io.ByteArrayInputStream
import java.io.InputStream

class Minio(
    endpoint: String = DEFAULT_ENDPOINT,
    accessKey: String = DEFAULT_ACCESS_KEY,
    secretKey: String = DEFAULT_SECRET_KEY
) : StorageApi {
    val client: MinioClient = MinioClient(endpoint, accessKey, secretKey).also {
        it.ignoreCertCheck()
    }

    override fun createBucketIfNotExists(bucketName: String) {
        if (!client.bucketExists(bucketName)) {
            client.makeBucket(bucketName)
            client.setBucketPolicy(
                bucketName, "{\n" +
                        "  \"Version\":\"2012-10-17\",\n" +
                        "  \"Statement\":[\n" +
                        "    {\n" +
                        "      \"Sid\":\"AddPerm\",\n" +
                        "      \"Effect\":\"Allow\",\n" +
                        "      \"Principal\": \"*\",\n" +
                        "      \"Action\":[\"s3:GetObject\"],\n" +
                        "      \"Resource\":[\"arn:aws:s3:::$bucketName/*\"]\n" +
                        "    }\n" +
                        "  ]\n" +
                        "}"
            )
        }
    }

    override fun deleteBucket(bucketName: String) {
        if (!client.bucketExists(bucketName)) {
            return
        }

        emptyBucket(bucketName)

        try {
            client.removeBucket(bucketName)
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    fun emptyBucket(bucketName: String) {
        if (!client.bucketExists(bucketName)) {
            return
        }

        var attemptsLeft = 20

        while (attemptsLeft-- > 0) {
            try {
                val objects = client.listObjects(bucketName)
                    .map { it.get().objectName() }
                if (objects.isEmpty()) {
                    break
                }

                objects.asIterable()
                    .let {
                        client.removeObjects(bucketName, it)
                    }.joinToString { it.get().message() }
                    .run {
                        if (isNotEmpty()) {
                            throw RuntimeException(this)
                        }
                    }
            } catch (e: XmlPullParserException) {
                println(e.message)
            }
        }
    }

    override fun storeObject(
        bucketName: String, objectName: String,
        data: ByteArray, mime: String
    ) {
        val inputStream = ByteArrayInputStream(data)
        client.putObject(bucketName, objectName, inputStream, data.size.toLong(), mime)
    }

    override fun getObject(bucketName: String, objectName: String): InputStream =
        client.getObject(bucketName, objectName)

    override fun deleteObject(bucketName: String, objectName: String) = client.removeObject(bucketName, objectName)

    companion object {
        const val DEFAULT_ENDPOINT = "https://hoernli-5.maas:30900"
        const val DEFAULT_ACCESS_KEY = "access"
        const val DEFAULT_SECRET_KEY = "secretkey"
    }
}
