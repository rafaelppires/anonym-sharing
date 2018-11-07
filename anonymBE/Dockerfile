FROM sebvaucher/sgx-base:sgx_2.3.1

COPY . ./
RUN cd ./src/SGX-OpenSSL/OpenSSL_SGX && ./sgx_openssl_setup.sh && cd - && make

CMD ["./bin/anonymbe"]

