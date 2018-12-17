package ch.unine.anonymbe.api

import okhttp3.ConnectionPool
import okhttp3.ConnectionSpec
import okhttp3.OkHttpClient
import okhttp3.RequestBody
import retrofit2.Call
import retrofit2.Retrofit
import retrofit2.converter.moshi.MoshiConverterFactory
import retrofit2.create
import retrofit2.http.*
import java.security.SecureRandom
import java.security.cert.X509Certificate
import java.util.concurrent.TimeUnit
import javax.net.ssl.SSLContext
import javax.net.ssl.X509TrustManager

interface AdminApi {
    @POST("access/user")
    fun createUser(@Body user: User): Call<UserResult>

    @HTTP(method = "DELETE", path = "access/user", hasBody = true)
    fun deleteUser(@Body user: User): Call<Result>

    @POST("access/group")
    fun createGroup(@Body group: UserGroup): Call<Result>

    @PUT("access/usergroup")
    fun addUserToGroup(@Body userGroup: UserGroup): Call<Result>

    @HTTP(method = "DELETE", path = "access/usergroup", hasBody = true)
    fun deleteUserFromGroup(@Body userGroup: UserGroup): Call<Result>

    @DELETE("access/all")
    fun deleteAllData(): Call<Result>
}

interface UserApi {
    @POST("verifier/envelope")
    fun getEnvelope(@Body bucket: Bucket): Call<EnvelopeResult>

    @POST("verifier/usergroup")
    fun isUserPartOfGroup(@Body userGroup: UserGroup): Call<BelongsResult>
}

interface WriterProxyApi {
    @PUT("{bucket}/{filename}")
    @Headers("Content-Type: application/octet-stream")
    fun uploadFile(@Path("bucket") bucketName: String, @Path("filename") filename: String, @Body data: RequestBody): Call<Unit>
}

object Api {
    private val trustAllCertificatesManager = object : X509TrustManager {
        override fun checkClientTrusted(chain: Array<out X509Certificate>?, authType: String?) = Unit

        override fun checkServerTrusted(chain: Array<out X509Certificate>?, authType: String?) = Unit

        override fun getAcceptedIssuers(): Array<X509Certificate> = emptyArray()
    }

    val okHttpClient: OkHttpClient = OkHttpClient.Builder()
        .connectTimeout(10, TimeUnit.SECONDS)
        .connectionPool(ConnectionPool(15, 10, TimeUnit.SECONDS))
        .sslSocketFactory(SSLContext.getInstance("TLS").also {
            it.init(null, arrayOf(trustAllCertificatesManager), SecureRandom())
        }.socketFactory, trustAllCertificatesManager)
        .hostnameVerifier { _, _ -> true }
        .connectionSpecs(listOf(ConnectionSpec.RESTRICTED_TLS))
        /* Log all requests if needed
        .addInterceptor(HttpLoggingInterceptor().apply {
            level = HttpLoggingInterceptor.Level.BODY
        })
        */
        .build()

    inline fun <reified T : Any> build(url: String = DEFAULT_URL): T = Retrofit.Builder()
        .baseUrl(url)
        .addConverterFactory(MoshiConverterFactory.create())
        .client(okHttpClient)
        .build()
        .create()

    const val RESULT_OK = "ok"
    const val RESULT_ERROR = "error"
    const val DEFAULT_URL = "https://hoernli-6.maas:30444/"
}
