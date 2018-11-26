FROM sebvaucher/sgx-base:sgx_2.3.1

COPY src/SGX-OpenSSL ./src/SGX-OpenSSL/
RUN cd ./src/SGX-OpenSSL/OpenSSL_SGX && ./sgx_openssl_setup.sh && make depend && make -j && cd -

COPY . ./
RUN make anonymbe

CMD ["./bin/anonymbe"]
