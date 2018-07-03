CC       :=gcc
CXX      :=g++
CFlags   := -std=c11
CXXFlags := -std=c++11

SGX_COMMONDIR  := $(realpath ../sgx_common)
SysName = anonymbe
EnclaveName := enclave_$(SysName)

BinDir=bin
SrcDir=src
ObjDir=obj
EXECUTABLES=$(addprefix $(BinDir)/, $(SysName))
EnclaveSources:=$(SrcDir)/enclave

App_Libs        := pthread sgx_uae_service sgx_urts
Enclave_Libs    := sgx_tstdc sgx_tservice sgx_uae_service
SharedEnclFlags := -DENCLAVED -DFAST_UNSAFE -DNOROLLBACKCHECK

include Makefile.sgx
include Makefile.in

EnclaveCXXIDirs  := $(SGX_SDK)/include/libcxx #$(SGX_SDK)/include/libcxx
Enclave_IncDirs  := $(SrcDir) $(EnclaveSources) $(SGX_SDK)/include \
                    $(SGX_SDK)/include/tlibc $(SGX_COMMONDIR)
EnclCInclude     := $(addprefix -I, $(Enclave_IncDirs))
EnclCXXInclude   := $(EnclCInclude) $(addprefix -I, $(EnclaveCXXIDirs))

NatvIncludeDirs  := $(SrcDir) $(EnclaveSources) $(SGX_COMMONDIR) $(SGX_SDK)/include
NatvInclude      := $(addprefix -I, $(NatvIncludeDirs))

NatvObjs         := $(addprefix $(ObjDir)/, $(EnclaveName)_u.o event_loop.o \
                         sgx_utils_rp_u.o $(SysName)_ocalls.o \
                         sgx_initenclave_u.o \
                         sgx_errlist_u.o)
EnclAllObjs      := $(addprefix $(ObjDir)/, $(EnclaveName)_t.o \
                         ecalls_$(SysName)_t.o anonymbe_service_t.o \
                         sgx_cryptoall_t.o libc_proxy_t.o file_mock_t.o )

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
	@$(call run_and_test,$(SGX_ENCLAVE_SIGNER) sign -enclave $< -config $(EnclaveSources)/$(EnclaveName).config.xml -out $@ -key $(EnclaveSources)/enclave-key.pem > /dev/null 2>&1,"Sign SGX")

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

$(ObjDir)/%_t.o : $(EnclaveSources)/%.cpp | $(ObjDir)
	@$(call run_and_test,$(CXX) -c $< -o $@ $(Encl_CXXFlags) $(EnclCXXInclude),"CXX SGX")

$(ObjDir)/%_t.o : $(SrcDir)/%.cpp | $(ObjDir)
	@$(call run_and_test,$(CXX) -c $< -o $@ $(Encl_CXXFlags) $(EnclCXXInclude),"CXX SGX")

$(ObjDir)/%_t.o : $(SGX_COMMONDIR)/%.cpp | $(ObjDir)
	@$(call run_and_test,$(CXX) -c $< -o $@ $(Encl_CXXFlags) $(EnclCXXInclude),"CXX SGX")

################################################################################

.PHONY: clean all

clean:
	@bash -c "rm -rf $(EnclaveSources)/*_{t,u}.{c,h} $(BinDir) $(ObjDir)"

