#include <anonymbe_service.h>
#include <iostream>
#include <tls_config.h>
#include <memory_database.h>

class MyConnection : public IncomeConnection {
   public:
    bool &print() { return p; }

    int send(const char *buff, size_t len) {
        if (p) std::cout << std::string(buff, len) << std::endl;
    }

   private:
    bool p;
};

AnonymBE<MemDatabase> anonymbe;
long unsigned requests_received = 0;
void process_input(IncomeConnection &conn, const char *buff, size_t len) {
    try {
        std::string input(buff, len), response;
        Request req;

        Http1Decoder &decoder = conn.decoder();
        decoder.addChunk(input);
        while (decoder.requestReady()) {
            if (++requests_received % 100000 == 0)
                printf("%luk Requests\n", requests_received / 1000);
            req = decoder.getRequest();
            response = anonymbe.process_input(req).toString();
            ssize_t ret;
            int r = conn.send(response.data(), response.size());
        }
    } catch (const std::exception &e) {
        printf("Error: [%s]\n", e.what());
    } catch (...) {
        printf(":_(   o.O   ;_;\n");
    }
}

RequestBuilder put =
    RequestBuilder().protocol(HttpStrings::http11).method("PUT");

RequestBuilder post =
    RequestBuilder().protocol(HttpStrings::http11).method("POST");

RequestBody usergroup(int g, int u) {
    RequestBody data;
    data.append("{\"user_id\": \"user" + std::to_string(u) +
                "\",\"group_name\": \"group" + std::to_string(g) + "\"}");
    return data;
}

std::string make_usergroup_req(RequestBuilder &r, int gid, int uid) {
    r.url(UrlBuilder::parse("https://localhost/access/usergroup"));
    r.body(std::move(usergroup(gid, uid)));
    return r.build().toString();
}

std::string make_group_req(RequestBuilder &r, int gid, int uid) {
    r.url(UrlBuilder::parse("https://localhost/access/group"));
    r.body(std::move(usergroup(gid, uid)));
    return r.build().toString();
}

std::string make_user_req(RequestBuilder &r, int i) {
    r.url(UrlBuilder::parse("https://localhost/access/user"));
    RequestBody data;
    data.append("{\"user_id\": \"user" + std::to_string(i) + "\"}");
    r.body(std::move(data));
    return r.build().toString();
}

std::string make_envelope_req(RequestBuilder &r, int i,
                              const std::string &key) {
    r.url(UrlBuilder::parse("https://localhost/verifier/envelope"));
    RequestBody data;
    data.append("{\"group_id\": \"group" + std::to_string(i) +
                "\", \"bucket_key\": \"" + Crypto::b64_encode(key) + "\" }");
    r.body(std::move(data));
    return r.build().toString();
}

void doreq(IncomeConnection &conn, const std::string &request) {
    process_input(conn, request.data(), request.size());
}

int main() {
    try {
        Arguments args;
        args.port = 1;
        args.mongo[0] = '\0';
        anonymbe.init(&args);
        MyConnection conn;

        conn.print() = true;
        doreq(conn, make_user_req(post, 0));
        doreq(conn, make_group_req(post, 1, 0));

        conn.print() = false;
        for (int i = 1; i < 10001; ++i) {
            if (i % 1000 == 0) std::cout << i << std::endl;
            doreq(conn, make_user_req(post, i));
            doreq(conn, make_usergroup_req(put, 1, i));
        }  //*/

        conn.print() = false;
        doreq(conn, make_envelope_req(post, 1, "klsdfjaçlvk"));

    } catch (const std::exception &e) {
        printf("> Error: [%s]\n", e.what());
    }
}
