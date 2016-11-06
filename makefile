
ifdef DEBUG
DBG="debug"
endif

ifndef XRD_PATH
$(error XRD_PATH is not set, set XRD_PATH to your XRootD installation)
endif
all:XrdOpenLocal.so
 	
XrdOpenLocal.so: 
	g++ -g3 -fPIC  -I$(XRD_PATH)/include/xrootd  -I./src/ -c *.cc -std=c++11
	g++ -shared  -L$(XRD_PATH)/lib -Wl,-soname,XrdProxyPrefix.so.1,--export-dynamic -o XrdProxyPrefix.so *.o -lXrdUtils -lXrdCl
	
test: XrdOpenLocal.so
	@./test/xrdcp_DEFAULT.sh $(DBG)
	@./test/xrdcp_NODEFAULT.sh $(DBG)
	
clean:clean_o clean_lib clean_exe 

clean_o:
	@-rm  *.o 
clean_lib:
	@-rm *.so
clean_exe:
	@-rm -rf *.exe

.PHONY: test
