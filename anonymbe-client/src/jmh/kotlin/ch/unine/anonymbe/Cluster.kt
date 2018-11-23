package ch.unine.anonymbe

import ch.unine.anonymbe.Deployments.ANONYMBE_MEM_URL
import ch.unine.anonymbe.Deployments.ANONYMBE_MONGO_URL
import java.io.BufferedReader
import java.io.InputStreamReader

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
    fun runKubectl(args: String): List<String> {
        val process = ProcessBuilder("/usr/bin/kubectl --kubeconfig=/etc/kubernetes/admin.conf $args".split(' '))
            .redirectOutput(ProcessBuilder.Redirect.PIPE)
            .redirectError(ProcessBuilder.Redirect.PIPE)
            .start()

        if (process.waitFor() != 0) {
            val error = BufferedReader(InputStreamReader(process.errorStream))
            throw Exception("Error running kubectl ${error.readText()}")
        }

        return BufferedReader(InputStreamReader(process.inputStream)).readLines()
    }

    fun scaleAnonymBEService(deployment: Deployment, instances: Int) {
        val deploymentName = deployment.deploymentName
        println("Scaling $deploymentName to $instances replicas...")

        runKubectl("scale deployment --replicas=$instances $deploymentName")

        println("Waiting for scaling to happen...")
        var currentReady = -1
        var tries = 0
        while (currentReady != instances && tries < 20) {
            Thread.sleep(1000)
            try {
                val lines = runKubectl("get deployment $deploymentName -o custom-columns=ready:status.readyReplicas")
                val newCurrentReady = lines[1].toInt()
                if (newCurrentReady != currentReady) {
                    println("$newCurrentReady instances ready")
                    currentReady = newCurrentReady
                }
            } catch (e: Exception) {
                e.printStackTrace()
            }
            tries++
        }
        if (currentReady == instances) {
            println("Scaled!")
        } else {
            throw Exception("Error with scaling")
        }
    }
}
