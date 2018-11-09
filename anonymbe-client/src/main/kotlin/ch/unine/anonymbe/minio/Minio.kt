package ch.unine.anonymbe.minio

import io.minio.MinioClient

object Minio {
    private const val BUCKET = "newbucket3"
    private const val OBJECT = "testfile.txt"

    @JvmStatic
    fun main(args: Array<String>) {
        println("minio upload example with SDK")

        val client = MinioClient("http://hoernli-5.maas", 30900, "access", "secretkey")
        if (!client.bucketExists(BUCKET)) {
            client.makeBucket(BUCKET)
            client.setBucketPolicy(BUCKET, "{\n" +
                    "  \"Version\":\"2012-10-17\",\n" +
                    "  \"Statement\":[\n" +
                    "    {\n" +
                    "      \"Sid\":\"AddPerm\",\n" +
                    "      \"Effect\":\"Allow\",\n" +
                    "      \"Principal\": \"*\",\n" +
                    "      \"Action\":[\"s3:GetObject\"],\n" +
                    "      \"Resource\":[\"arn:aws:s3:::$BUCKET/*\"]\n" +
                    "    }\n" +
                    "  ]\n" +
                    "}")
        }

        val toUpload = "Hello world from Kotlin!\n".byteInputStream()

        client.putObject(BUCKET, OBJECT, toUpload, toUpload.available().toLong(), "text/plain")
    }
}
