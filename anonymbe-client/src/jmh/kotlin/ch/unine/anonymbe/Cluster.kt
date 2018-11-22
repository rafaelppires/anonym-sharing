package ch.unine.anonymbe

import ch.unine.anonymbe.Deployments.ANONYMBE_MEM_URL
import ch.unine.anonymbe.Deployments.ANONYMBE_MONGO_URL
import java.io.BufferedReader
import java.io.InputStreamReader
import java.util.concurrent.TimeUnit
import kotlin.concurrent.thread

object Deployments {
    const val ANONYMBE_MONGO_URL = "https://hoernli-6.maas:30444/"
    const val ANONYMBE_MEM_URL = "https://hoernli-6.maas:30445/"
}

enum class Deployment(val deploymentName: String, val apiUrl: String) {
    ANONYMBE_MONGO("anonymbe", ANONYMBE_MONGO_URL),
    ANONYMBE_MEM("anonymbe-mem", ANONYMBE_MEM_URL);

    companion object {
        fun fromUrl(url: String): Deployment {
            return values().find { it.apiUrl == url }
                ?: throw IllegalArgumentException("Deployment with URL $url not found")
        }
    }
}

object Cluster {
    fun scaleAnonymBEService(deployment: Deployment, instances: Int) {
        print("Scaling ${deployment.deploymentName} to $instances replicas...")

        Runtime.getRuntime()
            .exec("kubectl scale deployment --replicas=$instances ${deployment.deploymentName}")
            .also {
                thread {
                    val input = BufferedReader(InputStreamReader(it.inputStream))
                    while (true) {
                        input.readLine()?.let(::println) ?: break
                    }
                }
                println(" scaled!")
            }
            .waitFor(5, TimeUnit.SECONDS)
    }
}
