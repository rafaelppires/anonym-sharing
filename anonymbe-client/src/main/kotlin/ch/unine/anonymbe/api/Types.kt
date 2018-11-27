package ch.unine.anonymbe.api

import com.squareup.moshi.JsonClass
import retrofit2.Response

@JsonClass(generateAdapter = true)
data class UserGroup(
    val user_id: String,
    val group_name: String
)

@JsonClass(generateAdapter = true)
data class User(
    val user_id: String
)

@JsonClass(generateAdapter = true)
data class Bucket(
    val user_id: String,
    val group_id: String,
    val bucket_key: String
)

@JsonClass(generateAdapter = true)
open class Result(
    val result: String,
    /**
     * Error, kind
     */
    val msg: String?,
    /**
     * Error, explanation
     */
    val detail: String?,
    /**
     * Info when something is not an error, but is unusual
     */
    val info: String?
) {
    override fun toString(): String {
        return "Result(result='$result', msg=$msg, detail=$detail, info=$info)"
    }
}

@JsonClass(generateAdapter = true)
class EnvelopeResult(
    result: String, msg: String?, detail: String?, info: String?,
    /**
     * Base64
     */
    val ciphertext: String
) : Result(result, msg, detail, info) {
    override fun toString(): String {
        return "EnvelopeResult(ciphertext='$ciphertext') ${super.toString()}"
    }
}

@JsonClass(generateAdapter = true)
class UserResult(
    result: String, msg: String?, detail: String?, info: String?,
    val user_key: String
) : Result(result, msg, detail, info) {
    override fun toString(): String {
        return "UserResult(user_key='$user_key') ${super.toString()}"
    }
}

@Throws(Exception::class)
fun <T> Response<T>.throwExceptionIfNotSuccessful() {
    if (!isSuccessful) {
        throw Exception("Unsuccessful API call. HTTP code ${code()}. ${errorBody()?.string()}")
    }
}

@Throws(Exception::class)
fun <T : Result> Response<T>.throwExceptionIfNotReallySuccessful() {
    val body: String? = if (isSuccessful) {
        body()?.toString()
    } else {
        errorBody()?.string()
    }
    if (!isReallySuccessful) {
        throw Exception("Unsuccessful API call. HTTP code ${code()}. Body: $body")
    }
}

val <T : Result> Response<T>.isReallySuccessful: Boolean
    get() = isSuccessful && body()?.result == Api.RESULT_OK
