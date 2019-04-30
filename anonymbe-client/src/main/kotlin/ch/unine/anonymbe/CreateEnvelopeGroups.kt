package ch.unine.anonymbe

import ch.unine.anonymbe.api.*

/**
 * Creates users and groups to be used by the EnvelopeBenchmark
 */
fun main(args: Array<String>) {
    val service = Api.build<AdminApi>(args.getOrElse(0) { Api.DEFAULT_URL })

    for (i in 1..(MAX_MAGNITUDE + ADDITIONAL_USERS)) {
        try {
            service.createUser(User("$USER_NAME_PREFIX$i")).execute().throwExceptionIfNotReallySuccessful()
        } catch (e: Exception) {
            if (e.message?.contains("Attempt to add an existing user") == false) {
                throw e
            }
        }
        if (i % 1000 == 0) {
            println("At user $i")
        }
    }
    println("Users created")

    var magnitude = 1
    while (magnitude <= MAX_MAGNITUDE) {
        println("Creating group of $magnitude users")
        service
            .createGroup(UserGroup("${USER_NAME_PREFIX}1", "$GROUP_NAME_PREFIX$magnitude"))
            .execute()
            .throwExceptionIfNotReallySuccessful()

        for (i in 2..magnitude) {
            println("At user $i")
            var success = false
            while (!success) {
                try {
                    service
                        .addUserToGroup(UserGroup("$USER_NAME_PREFIX$i", "$GROUP_NAME_PREFIX$magnitude"))
                        .execute()
                        .throwExceptionIfNotReallySuccessful()
                    success = true
                } catch (e: Exception) {
                    e.printStackTrace()
                    Thread.sleep(1000)
                }
            }
        }
        println("Group created")

        magnitude *= 10
    }
}

const val MAX_MAGNITUDE = 10_000
const val ADDITIONAL_USERS = 5_000
const val USER_NAME_PREFIX = "envelope-user-"
const val GROUP_NAME_PREFIX = "envelope-group-"
