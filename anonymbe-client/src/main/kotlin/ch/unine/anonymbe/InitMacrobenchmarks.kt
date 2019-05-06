package ch.unine.anonymbe

import ch.unine.anonymbe.api.*
import ch.unine.anonymbe.storage.Minio

/**
 * Puts the system in the right state to execute YCSB-based macrobenchmarks
 */
fun main(args: Array<String>) {
    println("Emptying bucket $GROUP_NAME")
    Minio().emptyBucket(GROUP_NAME)
    println("Bucket is empty")

    if (args.getOrNull(0) == "bucketonly") {
        System.exit(0)
        return
    }

    val api: AdminApi = Api.build()
    println("Removing all users from AdminService")
    api.deleteAllData().execute().throwExceptionIfNotReallySuccessful()
    println("No more users")

    println("Adding $NB_FIXED_USERS fixed users, all part of group \"$GROUP_NAME\"")
    run {
        val username = "${FIXED_USERS_PREFIX}0"
        api.createUser(User(username)).execute().throwExceptionIfNotReallySuccessful()
        api.createGroup(UserGroup(username, GROUP_NAME)).execute().throwExceptionIfNotReallySuccessful()
    }

    for (i in 1 until NB_FIXED_USERS) {
        val username = "$FIXED_USERS_PREFIX$i"
        api.createUser(User(username)).execute().throwExceptionIfNotReallySuccessful()
        api.addUserToGroup(UserGroup(username, GROUP_NAME)).execute().throwExceptionIfNotReallySuccessful()
    }
    println("Users added")

    println("\n** Now, load the rest of the data using YCSB's load function **")
    System.exit(0)
}

private const val GROUP_NAME = "field0"
private const val NB_FIXED_USERS = 64
private const val FIXED_USERS_PREFIX = "macrouser"
