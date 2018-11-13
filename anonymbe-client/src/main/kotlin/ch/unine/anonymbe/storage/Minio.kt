package ch.unine.anonymbe.storage

import io.minio.MinioClient
import java.io.InputStream

class Minio(
    endpoint: String = DEFAULT_ENDPOINT, port: Int = DEFAULT_PORT,
    accessKey: String = DEFAULT_ACCESS_KEY, secretKey: String = DEFAULT_SECRET_KEY
) : StorageApi {
    private val client = MinioClient(endpoint, port, accessKey, secretKey)

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

    override fun storeObject(
        bucketName: String, objectName: String,
        data: InputStream, dataLength: Long,
        mime: String
    ) = client.putObject(bucketName, objectName, data, dataLength, mime)

    override fun getObject(bucketName: String, objectName: String): InputStream =
        client.getObject(bucketName, objectName)

    companion object {
        const val DEFAULT_ENDPOINT = "http://hoernli-5.maas"
        const val DEFAULT_PORT = 30900
        const val DEFAULT_ACCESS_KEY = "access"
        const val DEFAULT_SECRET_KEY = "secretkey"
    }
}
