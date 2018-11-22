package ch.unine.anonymbe

import ch.unine.anonymbe.Deployments.ANONYMBE_MEM_URL
import ch.unine.anonymbe.Deployments.ANONYMBE_MONGO_URL
import io.fabric8.kubernetes.client.DefaultKubernetesClient

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
    private val client
        get() = DefaultKubernetesClient()

    fun scaleAnonymBEService(deployment: Deployment, instances: Int) {
        print("Scaling anonymbe to $instances replicas...")
        client
            .apps()
            .deployments()
            .inNamespace(ANONYMBE_NAMESPACE)
            .withName(deployment.deploymentName)
            .scale(instances, true)
        client.close()
        println(" scaled!")
    }

    private const val ANONYMBE_NAMESPACE = "default"
}
