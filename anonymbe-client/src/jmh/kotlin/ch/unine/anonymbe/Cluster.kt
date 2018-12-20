package ch.unine.anonymbe

import ch.unine.anonymbe.Deployments.ANONYMBE_MEM_URL
import ch.unine.anonymbe.Deployments.ANONYMBE_MONGO_URL
import ch.unine.anonymbe.Deployments.NGINX_URL
import ch.unine.anonymbe.Deployments.WRITERPROXY_TOKEN_URL
import ch.unine.anonymbe.Deployments.WRITERPROXY_URL
import java.io.BufferedReader
import java.io.InputStreamReader

object Deployments {
    const val ANONYMBE_MONGO_URL = "https://hoernli-4.maas:30444/"
    const val ANONYMBE_MEM_URL = "https://hoernli-4.maas:30445/"
    const val WRITERPROXY_URL = "https://hoernli-4.maas:30555/"
    const val WRITERPROXY_TOKEN_URL = "https://hoernli-4.maas:30556/"
    const val MINIO_URL = "https://hoernli-5.maas:30900/"
    const val NGINX_URL = "https://hoernli-4.maas:30700"
}

enum class Deployment(val deploymentName: String, val apiUrl: String) {
    ANONYMBE_MONGO("anonymbe", ANONYMBE_MONGO_URL),
    ANONYMBE_MEM("anonymbe-mem", ANONYMBE_MEM_URL),
    WRITERPROXY("writerproxy", WRITERPROXY_URL),
    WRITERPROXY_TOKEN("writerproxy-token", WRITERPROXY_TOKEN_URL),
    NGINX("nginx", NGINX_URL);


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

    fun scaleService(deployment: Deployment, instances: Int) {
        val deploymentName = deployment.deploymentName
        println("Scaling $deploymentName to $instances replicas...")

        var currentReady = -1
        var tries = 0
        while (currentReady != instances && tries < 40) {
            Thread.sleep(2000)
            try {
                runKubectl("scale deployment --replicas=$instances $deploymentName")
                val lines = runKubectl("get deployment $deploymentName -o custom-columns=ready:status.readyReplicas")
                val newCurrentReady = lines[1].toIntOrNull() ?: 0
                if (newCurrentReady != currentReady) {
                    println("$newCurrentReady instances ready")
                    currentReady = newCurrentReady
                }
            } catch (e: Exception) {
                println(e.message)
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
