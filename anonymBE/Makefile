CC       :=gcc
CXX      :=g++
CFlags   := -std=c11
CXXFlags := -std=c++11
SharedCompFlags := -m64


SysName = anonymbe
EnclaveName := enclave_$(SysName)

BinDir=bin
SrcDir=src
ObjDir=obj

MONGODIR = src/mongo-sgx-client
MONGO_TRUSTDIR  = $(MONGODIR)/trusted
MONGO_UNTRUSTDIR  = $(MONGODIR)/untrusted

OpenSSLDir= $(realpath src/SGX-OpenSSL/)
OpenSSLSGXDir=$(OpenSSLDir)/OpenSSL_SGX/
OSLLEnclaveW=$(OpenSSLDir)/Wrappers/Enclave
OSLLAppW=$(OpenSSLDir)/Wrappers/App
MinioClientDir=$(realpath src/minioclient-cpp)

ExtraExec=writeproxy
EnclaveSources:=$(SrcDir)/enclave

App_Libs        := pthread sgx_uae_service sgx_urts
Enclave_Libs    := sgx_tstdc sgx_tservice sgx_uae_service
SharedCompFlags += -DTLS_REQUESTS
SharedEnclFlags := -DENCLAVED -DSGX_OPENSSL
ifeq ($(MEMDATABASE), 1)
    SharedEnclFlags += -DMEMDATABASE
    SharedCompFlags += -DMEMDATABASE
else
    SharedEnclFlags += -DABEMONGO
endif
ifeq ($(NATIVE), 1)
    SharedCompFlags += -DNATIVE -DSGX_OPENSSL
endif

include Makefile.sgx
include Makefile.in
EDLSearchPaths+=$(OSLLEnclaveW)

EnclaveCXXIDirs  := $(SGX_SDK)/include/libcxx #$(SGX_SDK)/include/libcxx
Enclave_IncDirs  := $(SrcDir) $(EnclaveSources) $(SGX_SDK)/include \
                    $(SGX_SDK)/include/tlibc $(SGX_COMMONDIR) \
                    $(OpenSSLDir)/OpenSSL_SGX/include \
                    $(SGX_COMMONDIR)/enclave_include
NatvIncludeDirs  := $(SrcDir) $(EnclaveSources) $(SGX_COMMONDIR) \
                    $(SGX_SDK)/include
NatvObjs         := $(addprefix $(ObjDir)/, $(EnclaveName)_u.o event_loop.o \
                         sgx_utils_rp_u.o $(SysName)_ocalls.o \
                         sgx_initenclave_u.o sgx_errlist_u.o \
                         my_wrappers_impl_u.o )
EnclAllObjs      := $(addprefix $(ObjDir)/, $(EnclaveName)_t.o \
                         httpcommon_t.o ecalls_$(SysName)_t.o \
                         sgx_cryptoall_t.o libc_proxy_t.o file_mock_t.o \
                         ssl_wrappers_t.o my_wrappers_t.o tls_config_t.o \
                         httpresponse_t.o httprequest_t.o http1decoder_t.o \
                         httpheaders_t.o httpurl_t.o \
                         stringutils_t.o)
EnclExtLibs      := $(addprefix $(OpenSSLSGXDir), libssl.a libcrypto.a )
WriterProxyObjs  := $(addprefix $(ObjDir)/,enclave_writeproxy_u.o \
                        sgx_initenclave_u.o sgx_utils_rp_u.o \
                        event_loop_writeproxy_u.o sgx_errlist_u.o \
                        my_wrappers_impl_u.o writeproxy_ocalls_u.o)
WriterProxyEnclObjs := $(addprefix $(ObjDir)/, enclave_%_t.o ecalls_%_t.o \
                            tls_config_t.o ssl_wrappers_t.o \
                            my_wrappers_writeproxy_t.o libc_proxy_t.o \
                            file_mock_t.o http1decoder_t.o httprequest_t.o\
                            httpresponse_t.o httpheaders_t.o stringutils_t.o\
                            httpurl_t.o minioclient_t.o datetime_t.o \
                            inet_pton_ntop_t.o httpclient_t.o aws_signer_t.o\
                            httpcommon_t.o tcpconnection_t.o \
                            writerproxy_service_t.o) 

ifeq ($(MEMDATABASE), 1)
    EnclAllObjs += $(addprefix $(ObjDir)/, memory_database_t.o)
    EnclaveCXXIDirs += $(MONGO_TRUSTDIR)
#    EnclExtLibs := $(addprefix $(MONGO_TRUSTDIR)/lib/, libssl.a libcrypto.a)
else
    EnclaveCXXIDirs += $(MONGO_TRUSTDIR) \
                       $(MONGO_TRUSTDIR)/include/libmongoc-1.0 \
                       $(MONGO_TRUSTDIR)/include/libbson-1.0 \
                       $(SGX_COMMONDIR)/enclave_include

    EnclAllObjs += $(addprefix $(ObjDir)/, MongoDatabase_t.o pthread_t.o \
                                           inet_pton_ntop_t.o )
    EnclExtLibs := $(addprefix $(MONGO_TRUSTDIR)/lib/, libmongoc-static-1.0.a \
                                                       libbson-static-1.0.a \
                                                       libssl.a libcrypto.a )
#   EnclExtLibs := $(addprefix $(MONGO_TRUSTDIR)/lib/, \
#                                                libmongoc-static-1.0-debug.a  \
#                                                libbson-static-1.0-debug.a \
#                                                libssl.a libcrypto.a ) 
endif

EnclaveLFlags := $(EnclExtLibs) $(EnclaveLFlags)
EnclCInclude     := $(addprefix -I, $(Enclave_IncDirs))
EnclCXXInclude   := $(EnclCInclude) $(addprefix -I, $(EnclaveCXXIDirs) $(MinioClientDir))
NatvInclude      := $(addprefix -I, $(NatvIncludeDirs))

ifeq ($(DEBUG), 1)
    Encl_CFlags   += -g
    Encl_CXXFlags += -g
    Natv_CXXFlags += -g
    Natv_CFlags   += -g
endif

ifeq ($(NATIVE), 1)
$(SysName) $(ExtraExec) : % : $(BinDir)/%
    NatvObjs:=$(filter-out obj/enclave_anonymbe_u.o obj/sgx_initenclave_u.o \
                obj/sgx_errlist_u.o obj/my_wrappers_impl_u.o, $(NatvObjs))
    EnclAllObjs:=$(EnclAllObjs:%_t.o=%_u.o)
    EnclAllObjs:=$(filter-out obj/enclave_anonymbe_u.o obj/libc_proxy_u.o \
                    obj/file_mock_u.o obj/ssl_wrappers_u.o obj/my_wrappers_u.o,\
                    $(EnclAllObjs))
    NatvObjs+=$(EnclAllObjs)
    WriterProxyObjs:=$(filter-out obj/enclave_writeproxy_u.o \
            obj/sgx_initenclave_u.o obj/sgx_errlist_u.o \
            obj/my_wrappers_impl_u.o, $(WriterProxyObjs))
    WriterProxyEnclObjs:=$(WriterProxyEnclObjs:%_t.o=%_u.o)
    WriterProxyEnclObjs:=$(filter-out obj/libc_proxy_u.o \
                    obj/my_wrappers_writeproxy_u.o obj/file_mock_u.o \
                    obj/inet_pton_ntop_u.o obj/ssl_wrappers_u.o \
                    obj/enclave_%_u.o, $(WriterProxyEnclObjs))
    WriterProxyObjs+=$(WriterProxyEnclObjs)
    NatvInclude+=$(EnclCXXInclude)
    NatvInclude:=$(sort $(NatvInclude))
    NatvInclude:=$(filter-out -I/opt/intel/sgxsdk/include \
                    -I/opt/intel/sgxsdk/include/tlibc\
                    -I/opt/intel/sgxsdk/include/libcxx\
                    -I$(SGX_COMMONDIR)/enclave_include\
                    -I$(OpenSSLDir)/OpenSSL_SGX/include\
                    -Isrc/mongo-sgx-client/trusted ,$(NatvInclude))
    App_Link_Flags+=-lssl -lcrypto
    App_Link_Flags:=$(filter-out -lsgx_uae_service -lsgx_urts, \
                        $(App_Link_Flags))
else
$(SysName) $(ExtraExec) : % : $(BinDir)/% $(BinDir)/enclave_%.signed.so
$(ObjDir)/$(SysName).o : $(ObjDir)/$(EnclaveName)_u.o
$(ObjDir)/writeproxy.o : $(EnclaveSources)/enclave_writeproxy_u.c
endif

############## UNTRUSTED #######################################################
all: $(SysName) $(ExtraExec)

$(BinDir)/writeproxy : $(BinDir)/% : $(ObjDir)/%.o $(WriterProxyObjs) | $(BinDir) 
	@$(call run_and_test,$(CXX) $(Natv_CXXFlags) -o $@ $^ $(App_Link_Flags),"Link")

$(BinDir)/$(SysName) : $(ObjDir)/$(SysName).o $(NatvObjs) | $(BinDir)
	@$(call run_and_test,$(CXX) $(Natv_CXXFlags) -o $@ $^ $(App_Link_Flags),"Link")

$(ObjDir)/%.o : $(SrcDir)/%.cpp | $(ObjDir)
	@$(call run_and_test,$(CXX) $(Natv_CXXFlags) $(NatvInclude) -c -o $@ $<,"CXX")

$(ObjDir)/%_u.o : $(EnclaveSources)/%.cpp | $(ObjDir)
	@$(call run_and_test,$(CXX) $(Natv_CXXFlags) $(NatvInclude) -c -o $@ $<,"CXX")

$(ObjDir)/%_u.o : $(MinioClientDir)/%.cpp | $(ObjDir)
	@$(call run_and_test,$(CXX) -DWRITERPROXY $(Natv_CXXFlags) $(NatvInclude) -c -o $@ $<,"CXX")

$(ObjDir)/%_writeproxy_u.o : $(SrcDir)/%.cpp | $(ObjDir)
	@$(call run_and_test,$(CXX) -DWRITERPROXY $(Natv_CXXFlags) $(NatvInclude) -c -o $@ $<,"CXX")

$(ObjDir)/writeproxy_ocalls_u.o : $(SrcDir)/anonymbe_ocalls.cpp | $(ObjDir)
	@$(call run_and_test,$(CXX) -DWRITERPROXY $(Natv_CXXFlags) $(NatvInclude) -c -o $@ $<,"CXX")

$(ObjDir)/%_u.o : $(SGX_COMMONDIR)/%.cpp | $(ObjDir)
	@$(call run_and_test,$(CXX) $(Natv_CXXFlags) $(NatvInclude) -c -o $@ $<,"CXX")

$(EnclaveSources)/%_u.c : $(EnclaveSources)/%.edl $(SGX_EDGER8R)
	@echo $(WriterProxyObjs)
	@$(call run_and_test,cd $(dir $@) && $(SGX_EDGER8R) --untrusted ./$(notdir $<) $(addprefix --search-path ,. $(EDLSearchPaths)),"Edger SGX")

$(ObjDir)/%_u.o : $(EnclaveSources)/%_u.c | $(ObjDir)
	@$(call run_and_test,$(CC) $(Natv_CFlags) $(NatvInclude) -c $< -o $@,"CC")

$(BinDir) $(ObjDir):
	@mkdir -p $@

############## TRUSTED #########################################################
$(BinDir)/%.signed.so : $(ObjDir)/%.so | $(BinDir)
	$(eval EName := $(patsubst %.so, %, $(notdir $<)))
	@$(call run_and_test,$(SGX_ENCLAVE_SIGNER) sign -enclave $< -config $(EnclaveSources)/$(EName).config.xml -out $@ -key $(EnclaveSources)/enclave-key.pem -ignore-init-sec-error > /dev/null 2>&1,"Sign SGX")

$(ObjDir)/enclave_writeproxy.so : $(ObjDir)/enclave_%.so : $(WriterProxyEnclObjs) | $(ObjDir)
	@$(call run_and_test,$(CXX) $^ -o $@ $(EnclaveLFlags),"Link SGX")

$(ObjDir)/$(EnclaveName).so : $(EnclAllObjs) | $(ObjDir)
	@$(call run_and_test,$(CXX) $^ -o $@ $(EnclaveLFlags),"Link SGX")

$(EnclaveSources)/%_t.c : $(EnclaveSources)/%.edl $(SGX_EDGER8R)
	@$(call run_and_test,cd $(dir $@) && $(SGX_EDGER8R) --trusted ./$(notdir $<) $(addprefix --search-path ,. $(EDLSearchPaths)),"Edger SGX")

$(ObjDir)/%_t.o : $(EnclaveSources)/%_t.c | $(ObjDir)
	@$(call run_and_test,$(CC) -c $< -o $@ $(Encl_CFlags) $(EnclCInclude),"CC SGX")

$(ObjDir)/%_t.o : $(SGX_COMMONDIR)/libc_mock/%.c | $(ObjDir)
	@$(call run_and_test,$(CC) -c $< -o $@ $(Encl_CFlags) $(EnclCInclude),"CC SGX")

$(ObjDir)/%_t.o : $(SGX_COMMONDIR)/libc_mock/%.cpp | $(ObjDir)
	@$(call run_and_test,$(CXX) -c $< -o $@ $(Encl_CXXFlags) $(EnclCXXInclude),"CXX SGX")

$(ObjDir)/%_t.o : $(MinioClientDir)/%.cpp | $(ObjDir)
	@$(call run_and_test,$(CXX) -c $< -o $@ $(Encl_CXXFlags) $(EnclCXXInclude),"CXX SGX")

$(ObjDir)/%_t.o : $(EnclaveSources)/%.cpp | $(ObjDir)
	@$(call run_and_test,$(CXX) -c $< -o $@ $(Encl_CXXFlags) $(EnclCXXInclude),"CXX SGX")

$(ObjDir)/%_t.o : $(SGX_COMMONDIR)/%.cpp | $(ObjDir)
	@$(call run_and_test,$(CXX) -c $< -o $@ $(Encl_CXXFlags) $(EnclCXXInclude),"CXX SGX")

################# MONGO stuff ##################################################
$(ObjDir)/%_t.o : $(MONGO_TRUSTDIR)/%.cpp | $(ObjDir)
	@$(call run_and_test,$(CXX) -c $< -o $@ $(Encl_CXXFlags) $(EnclCXXInclude),"CXX SGX")

$(ObjDir)/%_t.o : $(MONGO_TRUSTDIR)/%.c | $(ObjDir)
	@$(call run_and_test,$(CC) -c $< -o $@ $(Encl_CFlags) $(EnclCInclude),"CC SGX")

$(ObjDir)/%_writeproxy_t.o : $(MONGO_TRUSTDIR)/%.c | $(ObjDir)
	@$(call run_and_test,$(CC) -c $< -o $@ -DWRITERPROXY $(Encl_CFlags) $(EnclCInclude),"CC SGX")

$(ObjDir)/%_u.o : $(MONGO_UNTRUSTDIR)/%.cpp | $(ObjDir)
	@$(call run_and_test,$(CXX) $(Natv_CXXFlags) $(NatvInclude) -c -o $@ $<,"CXX")

################# OPENSSL stuff ################################################
$(ObjDir)/sgx_cryptoall_t.o : $(OpenSSLSGXDir)/libcrypto.a
$(OpenSSLSGXDir)/libcrypto.a : $(OpenSSLSGXDir)/include
	@make -C $(OpenSSLSGXDir) depend && make -C $(OpenSSLSGXDir)

$(OpenSSLSGXDir)/include :
	@cd $(OpenSSLSGXDir) && ./sgx_openssl_setup.sh

################################################################################

.PHONY: clean all $(SysName) $(ExtraExec)


clean:
	@bash -c "rm -rf $(EnclaveSources)/*_{t,u}.{c,h} $(BinDir) $(ObjDir) $(EnclaveSources)/Ocall_wrappers.{cpp,h} $(SrcDir)/Ocall_implements.{cpp,h}"

$(ObjDir)/ecalls_writeproxy_t.o : $(EnclaveSources)/enclave_writeproxy_t.c
$(ObjDir)/tls_config_t.o : $(EnclaveSources)/enclave_anonymbe_t.c
$(ObjDir)/ecalls_anonymbe_t.o : $(OpenSSLSGXDir)/include
$(EnclaveSources)/ecalls_anonymbe.cpp : $(EnclaveSources)/anonymbe_service.h
$(EnclaveSources)/anonymbe_service.h : $(EnclaveSources)/anonymbe_service.hpp

