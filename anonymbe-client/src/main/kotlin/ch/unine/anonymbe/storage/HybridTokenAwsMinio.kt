package ch.unine.anonymbe.storage

class HybridTokenAwsMinio(
    minioEndpoint: String = Minio.DEFAULT_ENDPOINT,
    writerProxyEndpoint: String = WriterProxy.DEFAULT_URL,
    access: String = Minio.DEFAULT_ACCESS_KEY,
    secret: String = Minio.DEFAULT_SECRET_KEY
): StorageApi {
    private val minio = Minio(minioEndpoint, access, secret)
    private val aws = TokenAws(minioEndpoint, writerProxyEndpoint)

    override fun createBucketIfNotExists(bucketName: String) = aws.createBucketIfNotExists(bucketName)

    override fun deleteBucket(bucketName: String) = minio.deleteBucket(bucketName)

    override fun storeObject(
        bucketName: String,
        objectName: String,
        data: ByteArray,
        mime: String
    ) = aws.storeObject(bucketName, objectName, data, mime)

    override fun getObject(bucketName: String, objectName: String) = minio.getObject(bucketName, objectName)
}
