CXXFLAGS:=--std=c++11 -DNATIVE -DSGX_OPENSSL
LINK:=-lssl -lcrypto
INCLUDES:=-I. -I.. -I../minioclient-cpp/ -I../sgx_common/
MINIO:=$(addprefix ../minioclient-cpp/,  httpcommon.cpp httpresponse.cpp httpheaders.cpp stringutils.cpp httprequest.cpp httpurl.cpp http1decoder.cpp)
FILES=profile.cpp memory_database.cpp ../sgx_common/sgx_cryptoall.cpp
OBJS:= $(MINIO:%.cpp=%.o) $(FILES:%.cpp=%.o)

ifeq ($(DEBUG),1)
    CXXFLAGS+=-g
endif

all: profile

profile: $(OBJS) 
	@echo Link $@ : $^
	@g++ -o $@ $^ ${LINK}

%.o : %.cpp
	@echo $@
	@g++ ${CXXFLAGS} -c $^ -o $@ ${INCLUDES}

.PHONY: clean

clean:
	rm -f $(OBJS)

