package ch.unine.anonymbe.storage

import ch.unine.anonymbe.api.Api
import ch.unine.anonymbe.api.WriterProxyApi
import ch.unine.anonymbe.api.throwExceptionIfNotSuccessful
import org.w3c.dom.Element
import software.amazon.awssdk.auth.credentials.AwsSessionCredentials
import software.amazon.awssdk.auth.credentials.StaticCredentialsProvider
import software.amazon.awssdk.regions.Region
import software.amazon.awssdk.services.s3.S3Client
import java.io.InputStream
import java.net.URI
import java.time.Instant
import javax.xml.parsers.DocumentBuilderFactory
import javax.xml.xpath.XPathFactory

/*
<?xml version="1.0" encoding="UTF-8"?>
<AssumeRoleWithClientGrantsResponse xmlns="https://sts.amazonaws.com/doc/2011-06-15/">
    <AssumeRoleWithClientGrantsResult>
        <AssumedRoleUser>
            <Arn></Arn>
            <AssumeRoleId></AssumeRoleId>
        </AssumedRoleUser>
        <Credentials>
            <AccessKeyId>7LKMI5N6JC0TUEGY10H8</AccessKeyId>
            <SecretAccessKey>RouF3PG+I+o8txOn+PaZ0NcLiuXS+WfJkYC+KK+z</SecretAccessKey>
            <Expiration>2018-12-17T16:52:52Z</Expiration>
            <SessionToken>eyJhbGciOiJIUzUxMiIsInR5cCI6IkpXVCJ9.eyJhY2Nlc3NLZXkiOiI3TEtNSTVONkpDMFRVRUdZMTBIOCIsImF1ZCI6Ing3bmZTNjhBODhGWjR5V09rTDdwU0o3eXZrSWEiLCJhenAiOiJ4N25mUzY4QTg4Rlo0eVdPa0w3cFNKN3l2a0lhIiwiZXhwIjoxNTQ1MDY1NTcyLCJpYXQiOjE1NDUwNjE5NzIsImlzcyI6Imh0dHBzOi8vbG9jYWxob3N0Ojk0NDMvb2F1dGgyL3Rva2VuIiwianRpIjoiMmEyMzgxZjctYzRmYS00ODdiLWE2NTEtZGNkNDlkMzA2NWE3IiwibmJmIjoxNTQ1MDYxOTcyLCJzdWIiOiJhZG1pbkBjYXJib24uc3VwZXIifQ.MDJ8nQtzbMq7zZKq1hti7rshF0uqTNLaaeP5m_rfGB3WECQcMv-fBd1EW4ou0z7F2Zm856rNT-PQWaO2XLP-Qg</SessionToken>
        </Credentials>
    </AssumeRoleWithClientGrantsResult>
    <ResponseMetadata></ResponseMetadata>
</AssumeRoleWithClientGrantsResponse>
 */


data class TempToken(
    val access: String,
    val secret: String,
    val session: String,
    val expiration: Instant
) {
    constructor(document: Element) : this(
        accessXpath.evaluate(document),
        secretXpath.evaluate(document),
        sessionXpath.evaluate(document),
        Instant.parse(expirationXpath.evaluate(document))
    )

    constructor(xmlResponse: InputStream) : this(
        DocumentBuilderFactory.newInstance().newDocumentBuilder().parse(xmlResponse).documentElement
    )

    val needsRenewal = expiration <= Instant.now().minusSeconds(10)

    companion object {
        private const val BASE_XPATH_EXPR =
            "/AssumeRoleWithClientGrantsResponse/AssumeRoleWithClientGrantsResult/Credentials"
        private const val ACCESS_XPATH_EXPR = "$BASE_XPATH_EXPR/AccessKeyId"
        private const val SECRET_XPATH_EXPR = "$BASE_XPATH_EXPR/SecretAccessKey"
        private const val SESSION_XPATH_EXPR = "$BASE_XPATH_EXPR/SessionToken"
        private const val EXPIRATION_XPATH_EXPR = "$BASE_XPATH_EXPR/Expiration"
        private val xpath = XPathFactory.newInstance().newXPath()
        private val accessXpath = xpath.compile(ACCESS_XPATH_EXPR)
        private val secretXpath = xpath.compile(SECRET_XPATH_EXPR)
        private val sessionXpath = xpath.compile(SESSION_XPATH_EXPR)
        private val expirationXpath = xpath.compile(EXPIRATION_XPATH_EXPR)
    }
}

class TokenAws(
    private val awsEndpoint: String = DEFAULT_ENDPOINT,
    writerProxyEndpoint: String = WriterProxy.DEFAULT_URL_TOKEN
) : Aws() {
    private val writerProxyApi: WriterProxyApi = Api.build(writerProxyEndpoint)
    private var token: TempToken? = null
    private var _client: S3Client? = null

    override val client: S3Client
        get() {
            val currentClient = _client
            val isNewToken = ensureToken()
            return if (!isNewToken && currentClient != null) {
                currentClient
            } else {
                val token = token!!
                val newClient = S3Client.builder()
                    .endpointOverride(URI.create(awsEndpoint))
                    .region(Region.US_EAST_1)
                    .serviceConfiguration { it.pathStyleAccessEnabled(true) }
                    .httpClient(httpClient)
                    .credentialsProvider(StaticCredentialsProvider.create(
                        AwsSessionCredentials.create(token.access, token.secret, token.session)
                    ))
                    .build()
                _client = newClient
                newClient
            }
        }

    @Synchronized
    private fun ensureToken(): Boolean {
        if (token?.needsRenewal != false) {
            token = writerProxyApi.getToken().execute().let { response ->
                response.throwExceptionIfNotSuccessful()
                TempToken(response.body()?.byteStream() ?: return@let null)
            } ?: throw IllegalStateException("Token is null")
            println("token = $token")
            return true
        }
        return false
    }
}
