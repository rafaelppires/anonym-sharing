package ch.unine.anonymbe.storage

import io.minio.MinioClient

class RegularMinio(
    endpoint: String = DEFAULT_ENDPOINT,
    accessKey: String = DEFAULT_ACCESS_KEY,
    secretKey: String = DEFAULT_SECRET_KEY
) : Minio() {
    override val client: MinioClient = MinioClient(endpoint, accessKey, secretKey).also {
        it.ignoreCertCheck()
    }

    companion object {
        const val DEFAULT_ACCESS_KEY = "access"
        const val DEFAULT_SECRET_KEY = "secretkey"
    }
}
