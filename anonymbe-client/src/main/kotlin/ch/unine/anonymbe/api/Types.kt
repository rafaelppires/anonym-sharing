package ch.unine.anonymbe.api

import com.squareup.moshi.JsonClass

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
    val bucket_id: String,
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
)

@JsonClass(generateAdapter = true)
class EnvelopeResult(
    result: String, msg: String?, detail: String?, info: String?,
    /**
     * Base64
     */
    val ciphertext: String
) : Result(result, msg, detail, info)

@JsonClass(generateAdapter = true)
class UserResult(
    result: String, msg: String?, detail: String?, info: String?,
    val user_key: String
) : Result(result, msg, detail, info)
