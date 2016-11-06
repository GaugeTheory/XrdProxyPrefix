/********************************************************************************
 *    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    *
 *                                                                              *
 *              This software is distributed under the terms of the             * 
 *         GNU Lesser General Public Licence version 3 (LGPL) version 3,        *  
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/

#include "XrdProxyPrefix.hh"
#include <exception>
#include <cstdlib>
#include <string>
#include <utility>
#include "XrdCl/XrdClUtils.hh"
#include <assert.h>
using namespace XrdCl;
XrdVERSIONINFO(XrdClGetPlugIn, ProxyPrefix);

namespace ProxyPrefix {
enum Mode {Local,Default,Undefined};
class ProxyPrefixFile : public XrdCl::FilePlugIn {


private:

	///@proxyPrefix The prefix that will be added to any root query that cannot use local available files
	static std::string proxyPrefix;
	std::string path;
	XrdCl::File xfile;
public:
	static void setProxyPrefix(std::string toProxyPrefix) {
		proxyPrefix=toProxyPrefix;
	}
	static void printInfo() {
		XrdCl::Log *log= XrdCl::DefaultEnv::GetLog();
		std::string ppc=proxyPrefix;
		log->Debug(1,std::string("XrdProxyPrefix: ").append(ppc).c_str());		
	}
	
	std::string AddPrefix(std::string url) {
		XrdCl::Log *log= XrdCl::DefaultEnv::GetLog();
		XrdCl::URL xUrl(url);
		std::string ppc=proxyPrefix;
		std::string newurl=std::string(xUrl.GetProtocol()).append(std::string(ppc)).append(url);
		std::stringstream out;
		out << "XrdProxyPrefix::setting  url:\"" <<url<<" to: "<<newurl<<"\""<<std::endl;
		log->Debug(1,out.str().c_str());
		this->path=newurl;
		return newurl;
	}

	//Constructor
	ProxyPrefixFile():xfile(false) { 
		XrdCl::Log *log= XrdCl::DefaultEnv::GetLog();
		log->Debug(1,"ProxyPrefixFile::ProxyPrefixFile");
	}


	//Destructor
	~ProxyPrefixFile() {
		XrdCl::Log *log= XrdCl::DefaultEnv::GetLog();
		log->Debug(1,"~ProxyPrefixFile::ProxyPrefixFile");
	}

	//Open()
	virtual XRootDStatus Open( const std::string &url,
	                           OpenFlags::Flags   flags,
	                           Access::Mode       mode,
	                           ResponseHandler   *handler,
	                           uint16_t           timeout ) {
		XrdCl::Log *log=XrdCl::DefaultEnv::GetLog();
		XRootDStatus* ret_st;
		auto newurl= AddPrefix(url);
		log->Debug(1,"ProxyPrefixFile::Open");
		return xfile.Open(AddPrefix(url),flags,mode,handler,timeout);
	}
	//Close
	virtual XRootDStatus Close(ResponseHandler *handler,uint16_t timeout) {
		return xfile.Close(handler,timeout);
	}
	//isOpen
	virtual bool IsOpen()  const    {
		return xfile.IsOpen();

	}
	//Stat
	virtual XRootDStatus Stat(bool force,ResponseHandler *handler,uint16_t timeout) {
		return xfile.Stat(force,handler,timeout);
	}

	virtual XRootDStatus Read(uint64_t offset,uint32_t length,
	                          void  *buffer,XrdCl::ResponseHandler *handler,
	                          uint16_t timeout ) {
		XrdCl::Log *log=XrdCl::DefaultEnv::GetLog();
		log->Debug(1,"ProxyPrefixFile::Read");
			assert(xfile.IsOpen()==true);
			return xfile.Read(offset,length,buffer,handler,timeout);
	}

	XRootDStatus Write( uint64_t         offset,
	                    uint32_t         size,
	                    const void      *buffer,
	                    ResponseHandler *handler,
	                    uint16_t         timeout = 0 ) {
		XrdCl::Log *log=XrdCl::DefaultEnv::GetLog();
		log->Debug(1,"ProxyPrefixFile::Write");
			assert(xfile.IsOpen()==true);
			return xfile.Write(offset,size,buffer,handler,timeout);
	}
};
std::string ProxyPrefixFile::proxyPrefix="UNSET";

class ProxyPrefixFs : public XrdCl::FileSystemPlugIn{
private:
	static string proxyPrefix;
public:
	std::string originalURL;
	XrdCl::FileSystem xfs;
	//addPrefix	
	std::string addPrefix(std::string url) {
		return proxyPrefix;
	}
	//prepURL
	std::string prepURL(std::string toadd) {
		return std::string("/x").append(originalURL);
	}

	static void setProxyPrefix(std::string toProxyPrefix) {
		proxyPrefix=toProxyPrefix;
	}
	//Constructor
	ProxyPrefixFs(std::string url):xfs(addPrefix(url),false) {
		originalURL=url;
		XrdCl::Log *log=XrdCl::DefaultEnv::GetLog();
		log->Debug(1,"ProxyPrefixFs::ProxyPrefixFs");

	}
	//Destructor
	~ProxyPrefixFs() {
	}


	virtual XRootDStatus Locate( const std::string &path,
	                             OpenFlags::Flags   flags,
	                             ResponseHandler   *handler,
	                             uint16_t           timeout ) {
		XrdCl::Log *log=XrdCl::DefaultEnv::GetLog();
		log->Debug(1,"ProxyPrefixFs::Locate");
		return xfs.Locate(prepURL(path),flags,handler,timeout);
	}


	//Constructor
	virtual XRootDStatus Stat( const std::string &path,
	                           ResponseHandler   *handler,
	                           uint16_t           timeout ) {
		XrdCl::Log *log = DefaultEnv::GetLog();
		log->Debug(1,"ProxyPrefixFs::Stat");
		return xfs.Stat(prepURL(path),handler,timeout);
	}
};

std::string ProxyPrefixFs::proxyPrefix="UNSET";
}
namespace PPFactory {

void ProxyPrefixFactory::loadDefaultConf(std::map<std::string,std::string>& config) {
	XrdCl::Log *log = DefaultEnv::GetLog();
	log->Debug( 1, "ProxyPrefixFactory::loadDefaultConf" );
	if(const char* env_p = std::getenv("XRD_DEFAULT_PLUGIN_CONF")) {
		std::string confFile=env_p;
		std::stringstream msg;
		log->Debug(1, std::string("XRD_DEFAULT_PLUGIN_CONF file is: ").append(env_p).c_str());

		Status st = XrdCl::Utils::ProcessConfig( config, confFile );
		if(config.size() ==0 )throw std::runtime_error("XrdProxyPrefix cannot be loaded as the default plugin since the config file does not seem to have any content");
		if( !st.IsOK() ) {
			return;
		}

		const char *keys[] = {  "lib", "enable", 0 };
		for( int i = 0; keys[i]; ++i ) {
			if( config.find( keys[i] ) == config.end() ) {
				return;
			}
		}

		//--------------------------------------------------------------------------
		// Attempt to load the plug-in config-file and place it into the config map
		//--------------------------------------------------------------------------
		std::string lib = config["lib"];
		std::string enable = config["enable"];
		if( enable == "false" ) {
			throw std::runtime_error("XrdProxyPrefix cannot be loaded as the default plugin, since \"enable\" is set to \"false\" in the file located at XRD_DEFAULT_PLUGIN__CONF");
		}
	} else {
		throw std::runtime_error("XrdProxyPrefix cannot be loaded as the default plugin, since XRD_DEFAULT_PLUGIN_CONF is not set in the environment");
	}
}

ProxyPrefixFactory::ProxyPrefixFactory( const std::map<std::string, std::string> &config ) :
	XrdCl::PlugInFactory() {
	XrdCl::Log *log = DefaultEnv::GetLog();
	log->Debug( 1, "ProxyPrefixFactory::Constructor" );
	if(config.find("proxyPrefix")!=config.end())ProxyPrefix::ProxyPrefixFile::setProxyPrefix(config.find("proxyPrefix")->second);
	if(config.find("proxyPrefix")!=config.end())ProxyPrefix::ProxyPrefixFs::setProxyPrefix(config.find("proxyPrefix")->second);
	if(config.size()==0) {
		std::map<std::string,std::string> defaultconfig;
		log->Debug(1,"config size is zero... This is a \"default plug-in call\" -> loading default config file @ XRD_DEFAULT_PLUGIN_CONF Environment Variable ");
		loadDefaultConf(defaultconfig);
		//load config for Fileplugin
		if(defaultconfig.find("proxyPrefix")!=defaultconfig.end())ProxyPrefix::ProxyPrefixFile::setProxyPrefix(defaultconfig.find("proxyPrefix")->second);
		//load config for Filesystemplugin
		if(defaultconfig.find("proxyPrefix")!=defaultconfig.end())ProxyPrefix::ProxyPrefixFs::setProxyPrefix(defaultconfig.find("proxyPrefix")->second);
	}
	ProxyPrefix::ProxyPrefixFile::printInfo();
}
ProxyPrefixFactory::~ProxyPrefixFactory() {
	XrdCl::Log *log = XrdCl::DefaultEnv::GetLog();
	log->Debug( 1, "ProxyPrefixFactory::~ProxyPrefixFactory");
}

XrdCl::FilePlugIn * ProxyPrefixFactory::CreateFile( const std::string &url ) {
	XrdCl::Log *log = XrdCl::DefaultEnv::GetLog();
	log->Debug( 1, "ProxyPrefixFactory::CreateFile");
	return static_cast<XrdCl::FilePlugIn *> (new ProxyPrefix::ProxyPrefixFile()) ;
}

XrdCl::FileSystemPlugIn * ProxyPrefixFactory::CreateFileSystem(const std::string &url) {
	XrdCl::Log *log = XrdCl::DefaultEnv::GetLog();
	log->Debug( 1, "ProxyPreficFactory::CreateFilesys");
	return static_cast<XrdCl::FileSystemPlugIn *> (new ProxyPrefix::ProxyPrefixFs(url)) ;

}
}
extern "C" {

	void *XrdClGetPlugIn(const void *arg) {
		const std::map<std::string, std::string> &pconfig = *static_cast <const std::map<std::string, std::string> *>(arg);
		void * plug= new  PPFactory::ProxyPrefixFactory(pconfig);
		return plug;
	}

}
