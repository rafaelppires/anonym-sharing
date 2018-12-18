#ifndef _WRITERPROXY_SERVICE_H_
#define _WRITERPROXY_SERVICE_H_

#include <writeproxy_args.h>
#include <minioclient.h>
#include <httpresponse.h>
#include <httpheaders.h>

class WriterProxy {
   public:
    WriterProxy();
    ~WriterProxy();
    void finish();
    int init(Arguments args);
    Response treat_request(const Request &request);

   private:
    Response forward(const Request &request);
    Response generate_token(const Request &request);
    bool make_bucket(const Request &request);
    bool check_user();
    std::string request_reply(const HttpUrl &endpoint, const Request &request);

    std::string tuser_, tpass_;
    HttpUrl isendpoint_, minioendpoint_, amendpoint_;
    TlsConnection amconnection_;
    MinioClient *minioClient;
    HeadersBuilder header_builder;
    ResponseBuilder response_builder;
};

#endif
