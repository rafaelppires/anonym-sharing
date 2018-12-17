# WSO2

Tried to Dockerize the deployment, but too complicated. Deployed on bare metal instead.

## Bare-metal

1. Install https://product-dist.wso2.com/downloads/identity-server/5.7.0/downloader/wso2is-linux-installer-x64-5.7.0.deb
2. `sudo patch -d / -p1 < identity.xml.patch`
3. `sudo patch -d / -p1 < catalina-server.xml.patch`
4. Start wso2 with `sudo wso2is-5.7.0 --start` (no feedback, wait around 2 minutes for it to start)
5. In the manager (https://<HOST>:9443/carbon)
    a. Add a "Service Provider"
    b. In "Inbound Authentication Configuration", add an "OAuth/OpenID Connect Configuration"
    c. As callback URL, write https://<MINIO_HOST:MINIO_PORT>/callback
    d. Give the "Client Key" and "Client Secret" as parameters to the writer proxy

