package ch.unine.anonymbe.storage

import software.amazon.awssdk.auth.credentials.AwsCredentials
import software.amazon.awssdk.regions.Region
import software.amazon.awssdk.services.s3.S3Client
import java.net.URI

class RegularAws(
    endpointUrl: String = DEFAULT_ENDPOINT,
    accessKey: String = DEFAULT_ACCESS_KEY,
    secretKey: String = DEFAULT_SECRET_KEY
) : Aws() {
    override val client: S3Client = S3Client.builder()
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

    companion object {
        private const val DEFAULT_ACCESS_KEY = "access"
        private const val DEFAULT_SECRET_KEY = "secretkey"
    }
}
