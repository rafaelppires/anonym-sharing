CC       :=gcc
CXX      :=g++
CFlags   := -std=c11
CXXFlags := -std=c++11


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

EXECUTABLES=$(addprefix $(BinDir)/, $(SysName))
EnclaveSources:=$(SrcDir)/enclave

App_Libs        := pthread sgx_uae_service sgx_urts
Enclave_Libs    := sgx_tstdc sgx_tservice sgx_uae_service
SharedEnclFlags := -DENCLAVED -DSGX_OPENSSL
ifeq ($(MEMDATABASE), 1)
    SharedEnclFlags += -DMEMDATABASE
else
    SharedEnclFlags += -DABEMONGO
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
                         ecalls_$(SysName)_t.o \
                         sgx_cryptoall_t.o libc_proxy_t.o file_mock_t.o \
                         ssl_wrappers_t.o my_wrappers_t.o )
EnclExtLibs      := $(addprefix $(OpenSSLSGXDir), libssl.a libcrypto.a )

ifeq ($(MEMDATABASE), 1)
    EnclAllObjs += $(addprefix $(ObjDir)/, memory_database_t.o)
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
EnclCXXInclude   := $(EnclCInclude) $(addprefix -I, $(EnclaveCXXIDirs))
NatvInclude      := $(addprefix -I, $(NatvIncludeDirs))

############## UNTRUSTED #######################################################
all: $(EXECUTABLES) $(BinDir)/$(EnclaveName).signed.so

$(ObjDir)/$(SysName).o : $(ObjDir)/$(EnclaveName)_u.o

$(BinDir)/$(SysName) : $(ObjDir)/$(SysName).o $(NatvObjs) | $(BinDir)
	@$(call run_and_test,$(CXX) $(CXXFlags) -o $@ $^ $(App_Link_Flags),"Link")

$(ObjDir)/%.o : $(SrcDir)/%.cpp | $(ObjDir)
	@$(call run_and_test,$(CXX) $(NatvInclude) $(CXXFlags) -c -o $@ $<,"CXX")

$(ObjDir)/%_u.o : $(SGX_COMMONDIR)/%.cpp | $(ObjDir)
	@$(call run_and_test,$(CXX) $(NatvInclude) $(CXXFlags) -c -o $@ $<,"CXX")

$(EnclaveSources)/$(EnclaveName)_u.c : $(EnclaveSources)/$(EnclaveName).edl $(SGX_EDGER8R)
	@$(call run_and_test,cd $(dir $@) && $(SGX_EDGER8R) --untrusted ./$(notdir $<) $(addprefix --search-path ,. $(EDLSearchPaths)),"Edger SGX")

$(ObjDir)/$(EnclaveName)_u.o : $(EnclaveSources)/$(EnclaveName)_u.c | $(ObjDir)
	@$(call run_and_test,$(CC) $(Natv_CFlags) $(NatvInclude) -c $< -o $@,"CC")

$(BinDir) $(ObjDir):
	@mkdir -p $@

############## TRUSTED #########################################################
$(BinDir)/%.signed.so : $(ObjDir)/%.so | $(BinDir)
	@$(call run_and_test,$(SGX_ENCLAVE_SIGNER) sign -enclave $< -config $(EnclaveSources)/$(EnclaveName).config.xml -out $@ -key $(EnclaveSources)/enclave-key.pem -ignore-init-sec-error > /dev/null 2>&1,"Sign SGX")

$(ObjDir)/$(EnclaveName).so : $(EnclAllObjs) | $(ObjDir)
	@$(call run_and_test,$(CXX) $^ -o $@ $(EnclaveLFlags),"Link SGX")

$(EnclaveSources)/$(EnclaveName)_t.c : $(EnclaveSources)/$(EnclaveName).edl $(SGX_EDGER8R)
	@$(call run_and_test,cd $(dir $@) && $(SGX_EDGER8R) --trusted ./$(notdir $<) $(addprefix --search-path ,. $(EDLSearchPaths)),"Edger SGX")

$(ObjDir)/$(EnclaveName)_t.o : $(EnclaveSources)/$(EnclaveName)_t.c | $(ObjDir)
	@$(call run_and_test,$(CC) -c $< -o $@ $(Encl_CFlags) $(EnclCInclude),"CC SGX")

$(ObjDir)/%_t.o : $(SGX_COMMONDIR)/libc_mock/%.c | $(ObjDir)
	@$(call run_and_test,$(CC) -c $< -o $@ $(Encl_CFlags) $(EnclCInclude),"CC SGX")

$(ObjDir)/%_t.o : $(SGX_COMMONDIR)/libc_mock/%.cpp | $(ObjDir)
	@$(call run_and_test,$(CXX) -c $< -o $@ $(Encl_CXXFlags) $(EnclCXXInclude),"CXX SGX")

#$(ObjDir)/%_t.o : $(OpenSSLDir)/Wrappers/Enclave/%.cpp | $(ObjDir)
#	@$(call run_and_test,$(CXX) -c $< -o $@ $(Encl_CXXFlags) $(EnclCXXInclude),"CXX SGX")

$(ObjDir)/%_t.o : $(EnclaveSources)/%.cpp | $(ObjDir)
	@$(call run_and_test,$(CXX) -c $< -o $@ $(Encl_CXXFlags) $(EnclCXXInclude),"CXX SGX")

#$(ObjDir)/%_t.o : $(SrcDir)/%.cpp | $(ObjDir)
#	@$(call run_and_test,$(CXX) -c $< -o $@ $(Encl_CXXFlags) $(EnclCXXInclude),"CXX SGX")

$(ObjDir)/%_t.o : $(SGX_COMMONDIR)/%.cpp | $(ObjDir)
	@$(call run_and_test,$(CXX) -c $< -o $@ $(Encl_CXXFlags) $(EnclCXXInclude),"CXX SGX")

################# MONGO stuff ##################################################
$(ObjDir)/%_t.o : $(MONGO_TRUSTDIR)/%.cpp | $(ObjDir)
	@$(call run_and_test,$(CXX) -c $< -o $@ $(Encl_CXXFlags) $(EnclCXXInclude),"CXX SGX")

$(ObjDir)/%_t.o : $(MONGO_TRUSTDIR)/%.c | $(ObjDir)
	@$(call run_and_test,$(CC) -c $< -o $@ $(Encl_CFlags) $(EnclCInclude),"CC SGX")

$(ObjDir)/%_u.o : $(MONGO_UNTRUSTDIR)/%.cpp | $(ObjDir)
	@$(call run_and_test,$(CXX) $(NatvInclude) $(CXXFlags) -c -o $@ $<,"CXX")

################# OPENSSL stuff ################################################
obj/sgx_cryptoall_t.o : $(OpenSSLSGXDir)/libcrypto.a
$(OpenSSLSGXDir)/libcrypto.a : $(OpenSSLSGXDir)/include
	@make -C $(OpenSSLSGXDir) depend && make -C $(OpenSSLSGXDir)

$(OpenSSLSGXDir)/include :
	@cd $(OpenSSLSGXDir) && ./sgx_openssl_setup.sh

################################################################################

.PHONY: clean all

clean:
	@bash -c "rm -rf $(EnclaveSources)/*_{t,u}.{c,h} $(BinDir) $(ObjDir) $(EnclaveSources)/Ocall_wrappers.{cpp,h} $(SrcDir)/Ocall_implements.{cpp,h}"

$(EnclaveSources)/ecalls_anonymbe.cpp : $(EnclaveSources)/anonymbe_service.h
$(EnclaveSources)/anonymbe_service.h : $(EnclaveSources)/anonymbe_service.hpp

