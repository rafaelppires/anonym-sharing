package ch.unine.anonymbe.storage

import io.minio.MinioClient
import org.xmlpull.v1.XmlPullParserException
import java.io.InputStream

abstract class Minio : StorageApi {
    protected abstract val client: MinioClient

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

        var attemptsLeft = 20

        while (attemptsLeft --> 0) {
            try {
                val objects = client.listObjects(bucketName)
                    .map { it.get().objectName() }
                if (objects.isEmpty()) {
                    break
                }

                objects.asIterable()
                    .let {
                        client.removeObject(bucketName, it)
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

        try {
            client.removeBucket(bucketName)
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    override fun storeObject(
        bucketName: String, objectName: String,
        data: InputStream, dataLength: Long,
        mime: String
    ) = client.putObject(bucketName, objectName, data, dataLength, mime)

    override fun getObject(bucketName: String, objectName: String): InputStream =
        client.getObject(bucketName, objectName)

    companion object {
        const val DEFAULT_ENDPOINT = "https://hoernli-5.maas:30900"
    }
}
