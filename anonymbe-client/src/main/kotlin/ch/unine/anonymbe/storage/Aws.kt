package ch.unine.anonymbe.storage

import software.amazon.awssdk.auth.credentials.AwsCredentials
import software.amazon.awssdk.core.sync.RequestBody
import software.amazon.awssdk.regions.Region
import software.amazon.awssdk.services.s3.S3Client
import software.amazon.awssdk.services.s3.model.BucketAlreadyOwnedByYouException
import java.net.URI

object Aws {
    private const val ENDPOINT = "http://hoernli-5.maas:30900"
    private const val BUCKET = "awsbucket2"
    private const val FILEPATH = "testfile.txt"

    @JvmStatic
    fun main(args: Array<String>) {
        val client = S3Client.builder()
            .endpointOverride(URI.create(ENDPOINT))
            .region(Region.US_EAST_1)
            .serviceConfiguration { it.pathStyleAccessEnabled(true) }
            .credentialsProvider {
                object : AwsCredentials {
                    override fun accessKeyId(): String = "access"

                    override fun secretAccessKey(): String = "secretkey"
                }
            }
            .build()

        try {
            client.createBucket {
                it.bucket(BUCKET)
            }
        } catch (_: BucketAlreadyOwnedByYouException) {}

        /* Does not work... Access denied.
        client.putBucketPolicy {
            it.bucket(BUCKET)
            it.policy(
                "{\n" +
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
                "}"
            )
        }*/*/ // Needs double comment ending for some reason.

        client.putObject(
            {
                it.bucket(BUCKET)
                it.contentType("text/plain")
                it.key(FILEPATH)
            },
            RequestBody.fromBytes("Hello AWS\n".toByteArray())
        )
    }
}
