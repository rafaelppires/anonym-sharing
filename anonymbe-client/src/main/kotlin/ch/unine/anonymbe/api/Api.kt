package ch.unine.anonymbe.api

import okhttp3.ConnectionSpec
import okhttp3.OkHttpClient
import retrofit2.Call
import retrofit2.Retrofit
import retrofit2.converter.moshi.MoshiConverterFactory
import retrofit2.http.Body
import retrofit2.http.DELETE
import retrofit2.http.POST
import retrofit2.http.PUT
import java.security.SecureRandom
import java.security.cert.X509Certificate
import javax.net.ssl.SSLContext
import javax.net.ssl.X509TrustManager
import kotlin.reflect.KClass

interface AdminApi {
    @POST("access/user")
    fun createUser(@Body user: User): Call<UserResult>

    @POST("access/group")
    fun createGroup(@Body group: UserGroup): Call<Result>

    @PUT("access/usergroup")
    fun addUserToGroup(@Body userGroup: UserGroup): Call<Result>

    @DELETE("access/usergroup")
    fun deleteUserFromGroup(@Body userGroup: UserGroup): Call<Result>
}

interface UserApi {
    @POST("verifier/envelope")
    fun getEnvelope(@Body bucket: Bucket): Call<EnvelopeResult>
}

object Api {
    fun <T : Any> build(apiType: KClass<T>, url: String = DEFAULT_URL): T {
        val trustAllCertificatesManager = object : X509TrustManager {
            override fun checkClientTrusted(chain: Array<out X509Certificate>?, authType: String?) = Unit

            override fun checkServerTrusted(chain: Array<out X509Certificate>?, authType: String?) = Unit

            override fun getAcceptedIssuers(): Array<X509Certificate> = emptyArray()
        }

        val okHttpClient = OkHttpClient.Builder()
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
        val retrofit = Retrofit.Builder()
            .baseUrl(url)
            .addConverterFactory(MoshiConverterFactory.create())
            .client(okHttpClient)
            .build()
        return retrofit
            .create(apiType.java)
    }

    const val RESULT_OK = "ok"
    const val RESULT_ERROR = "error"
    const val DEFAULT_URL = "https://hoernli-6.maas:30444/"
}
