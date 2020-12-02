/********************************************************************************
 *    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH *
 *                                                                              *
 *              This software is distributed under the terms of the *
 *         GNU Lesser General Public Licence version 3 (LGPL) version 3, *
 *                  copied verbatim in the file "LICENSE" *
 ********************************************************************************/

#include "XrdProxyPrefix.hh"
#include "XrdCl/XrdClUtils.hh"
#include <assert.h>
#include <cstdlib>
#include <exception>
#include <string>
#include <utility>
using namespace XrdCl;
XrdVERSIONINFO(XrdClGetPlugIn, ProxyPrefix);

namespace ProxyPrefix {
enum Mode { Local, Default, Undefined };
class ProxyPrefixFile : public XrdCl::FilePlugIn {
  private:
    static std::string proxyPrefix;
    XrdCl::File xfile;

  public:
    static void setProxyPrefix(std::string toProxyPrefix) { proxyPrefix = toProxyPrefix; }
    static void printInfo() {
        XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
        std::string ppc = proxyPrefix;
        log->Debug(1, std::string("XrdProxyPrefix: ").append(ppc).c_str());
    }

    std::string AddPrefix(std::string url) {
        XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
        XrdCl::URL xUrl(url);
        std::string ppc = proxyPrefix;
        if (xUrl.GetProtocol().compare("file") == 0)
            return url;
        std::string newurl = std::string(xUrl.GetProtocol()).append(std::string(ppc)).append(std::string(xUrl.GetPath()));
        std::stringstream out;
        out << "XrdProxyPrefix::setting  url:\"" << url << " to: " << newurl << "\"" << std::endl;
        log->Debug(1, out.str().c_str());
        return newurl;
    }

    ProxyPrefixFile() : xfile(false) {
        XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
        log->Debug(1, "ProxyPrefixFile::ProxyPrefixFile");
    }

    ~ProxyPrefixFile() {
        XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
        log->Debug(1, "~ProxyPrefixFile::ProxyPrefixFile");
    }

    virtual XRootDStatus Open(const std::string& url, OpenFlags::Flags flags, Access::Mode mode,
                              ResponseHandler* handler, uint16_t timeout) {
        XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
        XRootDStatus* ret_st;
        auto newurl = AddPrefix(url);
        log->Debug(1, "ProxyPrefixFile::Open");
        return xfile.Open(AddPrefix(url), flags, mode, handler, timeout);
    }
    virtual XRootDStatus Close(ResponseHandler* handler, uint16_t timeout) {
        return xfile.Close(handler, timeout);
    }
    virtual bool IsOpen() const { return xfile.IsOpen(); }
    virtual XRootDStatus Stat(bool force, ResponseHandler* handler, uint16_t timeout) {
        return xfile.Stat(force, handler, timeout);
    }

    virtual XRootDStatus Read(uint64_t offset, uint32_t length, void* buffer,
                              XrdCl::ResponseHandler* handler, uint16_t timeout) {
        XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
        log->Debug(1, "ProxyPrefixFile::Read");
        assert(xfile.IsOpen() == true);
        return xfile.Read(offset, length, buffer, handler, timeout);
    }

    XRootDStatus Write(uint64_t offset, uint32_t size, const void* buffer, ResponseHandler* handler,
                       uint16_t timeout = 0) {
        XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
        log->Debug(1, "ProxyPrefixFile::Write");
        assert(xfile.IsOpen() == true);
        return xfile.Write(offset, size, buffer, handler, timeout);
    }
};
std::string ProxyPrefixFile::proxyPrefix = "UNSET";

class ProxyPrefixFs : public XrdCl::FileSystemPlugIn {
  private:
    static string proxyPrefix;
    static XrdCl::URL targetURL;
    static int level;
    int mylevel;

  public:
    XrdCl::FileSystem xfs;

    std::string getProxyDecor(std::string path) {
        XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
        log->Debug(1, "ProxyPrefixFs::ProxyDecor");
        XrdCl::URL xURL(path);
        char buffer[2048];
        snprintf(buffer, 2048, "%s://%s", xURL.GetProtocol().c_str(), proxyPrefix.c_str());
        return buffer;
    }

    std::string prepURL(std::string path) {
        XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
        XrdCl::URL xURL(path);
        char buffer[2048];
        snprintf(buffer, 2048, "/%s://%s/", targetURL.GetProtocol().c_str(),
                 targetURL.GetHostId().c_str());
        return buffer;
    }

    static void setProxyPrefix(std::string toProxyPrefix) { proxyPrefix = toProxyPrefix; }

    ProxyPrefixFs(std::string url) : xfs((level == 0) ? getProxyDecor(url) : url, false) {
        XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
        log->Debug(1, "ProxyPrefixFs::ProxyPrefixFs");
        if (level == 0) {
            log->Debug(1, "Setting targetURL");
            targetURL = XrdCl::URL(url);
        }
        mylevel = level;
        level++;
    }

    ~ProxyPrefixFs() {}

    virtual XRootDStatus Locate(const std::string& path, OpenFlags::Flags flags,
                                ResponseHandler* handler, uint16_t timeout) {
        XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
        log->Debug(1, "ProxyPrefixFs::Locate");
        if (mylevel == 0)
            return xfs.Locate(prepURL(path), flags, handler, timeout);
        return xfs.Locate(path, flags, handler, timeout);
    }
    virtual XRootDStatus Truncate(const std::string& path, uint64_t size, ResponseHandler* handler,
                                  uint16_t timeout) {
        XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
        log->Debug(1, "ProxyPrefixFs::Truncate");
        if (mylevel == 0)
            return xfs.Truncate(prepURL(path), size, handler, timeout);
        return xfs.Truncate(path, size, handler, timeout);
    }
    virtual XRootDStatus Rm(const std::string& path, ResponseHandler* handler, uint16_t timeout) {
        XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
        log->Debug(1, "ProxyPrefixFs::Rm");
        if (mylevel == 0)
            return xfs.Rm(prepURL(path), handler, timeout);
        return xfs.Rm(path, handler, timeout);
    }
    virtual XRootDStatus MkDir(const std::string& path, MkDirFlags::Flags flags, Access::Mode mode,
                               ResponseHandler* handler, uint16_t timeout) {
        XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
        log->Debug(1, "ProxyPrefixFs::MkDir");
        if (mylevel == 0)
            return xfs.MkDir(prepURL(path), flags, mode, handler, timeout);
        return xfs.MkDir(path, flags, mode, handler, timeout);
    }
    virtual XRootDStatus RmDir(const std::string& path, ResponseHandler* handler,
                               uint16_t timeout) {
        XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
        log->Debug(1, "ProxyPrefixFs::RmDir");
        if (mylevel == 0)
            return xfs.RmDir(prepURL(path), handler, timeout);
        return xfs.RmDir(path, handler, timeout);
    }
    virtual XRootDStatus ChMod(const std::string& path, Access::Mode mode, ResponseHandler* handler,
                               uint16_t timeout) {
        XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
        log->Debug(1, "ProxyPrefixFs::ChMod");
        if (mylevel == 0)
            return xfs.ChMod(prepURL(path), mode, handler, timeout);
        return xfs.ChMod(path, mode, handler, timeout);
    }

    virtual XRootDStatus Ping(ResponseHandler* handler, uint16_t timeout) {
        XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
        log->Debug(1, "ProxyPrefixFs::Ping");
        return xfs.Ping(handler, timeout);
    }
    virtual XRootDStatus Query(QueryCode::Code queryCode, const Buffer& arg,
                               ResponseHandler* handler, uint16_t timeout) {

        XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
        log->Debug(1, "ProxyPrefixFs::Query");
        return xfs.Query(queryCode, arg, handler, timeout);
    }
    virtual XRootDStatus DirList(const std::string& path, DirListFlags::Flags flags,
                                 ResponseHandler* handler, uint16_t timeout) {
        XrdCl::Log* log = DefaultEnv::GetLog();
        log->Debug(1, "ProxyPrefixFs::Dirlist");
        if (mylevel == 0)
            return xfs.DirList(prepURL(path), flags, handler, timeout);
        return xfs.DirList(prepURL(path), flags, handler, timeout);
    }

    virtual XRootDStatus Stat(const std::string& path, ResponseHandler* handler, uint16_t timeout) {
        XrdCl::Log* log = DefaultEnv::GetLog();
        log->Debug(1, "ProxyPrefixFs::Stat");
        if (mylevel == 0)
            return xfs.Stat(prepURL(path), handler, timeout);
        return xfs.Stat(path, handler, timeout);
    }
    virtual XRootDStatus StatVFS(const std::string& path, ResponseHandler* handler,
                                 uint16_t timeout) {
        XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
        log->Debug(1, "ProxyPrefixFs::StatVFS");
        if (mylevel == 0)
            return xfs.StatVFS(prepURL(path), handler, timeout);
        return xfs.StatVFS(path, handler, timeout);
    }
    virtual XRootDStatus Protocol(ResponseHandler* handler, uint16_t timeout) {
        XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
        log->Debug(1, "ProxyPrefixFs::Protocol");
        return xfs.Protocol(handler, timeout);
    }
    virtual XRootDStatus SendInfo(const std::string& info, ResponseHandler* handler,
                                  uint16_t timeout) {
        XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
        log->Debug(1, "ProxyPrefixFs::Protocol");
        return xfs.SendInfo(info, handler, timeout);
    }
    virtual XRootDStatus Prepare(const std::vector<std::string>& fileList,
                                 PrepareFlags::Flags flags, uint8_t priority,
                                 ResponseHandler* handler, uint16_t timeout) {
        if (mylevel == 0) {
            std::vector<std::string> newList;
            for (auto it : fileList)
                newList.push_back(prepURL(it));
            return xfs.Prepare(newList, flags, priority, handler, timeout);
        }
        return xfs.Prepare(fileList, flags, priority, handler, timeout);
    }
};

int ProxyPrefixFs::level = 0;
std::string ProxyPrefixFs::proxyPrefix = "UNSET";
XrdCl::URL ProxyPrefixFs::targetURL;
} // namespace ProxyPrefix
namespace PPFactory {

void ProxyPrefixFactory::loadDefaultConf(std::map<std::string, std::string>& config) {
    XrdCl::Log* log = DefaultEnv::GetLog();
    log->Debug(1, "ProxyPrefixFactory::loadDefaultConf");
    if (const char* env_p = std::getenv("XRD_DEFAULT_PLUGIN_CONF")) {
        std::string confFile = env_p;
        std::stringstream msg;
        log->Debug(1, std::string("XRD_DEFAULT_PLUGIN_CONF file is: ").append(env_p).c_str());

        Status st = XrdCl::Utils::ProcessConfig(config, confFile);
        if (config.size() == 0)
            throw std::runtime_error("XrdProxyPrefix cannot be loaded as the default "
                                     "plugin since the config file does not seem to "
                                     "have any content");
        if (!st.IsOK()) {
            return;
        }

        const char* keys[] = { "lib", "enable", 0 };
        for (int i = 0; keys[i]; ++i) {
            if (config.find(keys[i]) == config.end()) {
                return;
            }
        }

        //--------------------------------------------------------------------------
        // Attempt to load the plug-in config-file and place it into the config map
        //--------------------------------------------------------------------------
        std::string lib = config["lib"];
        std::string enable = config["enable"];
        if (enable == "false") {
            throw std::runtime_error("XrdProxyPrefix cannot be loaded as the default "
                                     "plugin, since \"enable\" is set to \"false\" "
                                     "in the file located at "
                                     "XRD_DEFAULT_PLUGIN__CONF");
        }
    } else {
        throw std::runtime_error("XrdProxyPrefix cannot be loaded as the default "
                                 "plugin, since XRD_DEFAULT_PLUGIN_CONF is not set "
                                 "in the environment");
    }
}

ProxyPrefixFactory::ProxyPrefixFactory(const std::map<std::string, std::string>& config)
  : XrdCl::PlugInFactory() {
    XrdCl::Log* log = DefaultEnv::GetLog();
    log->Debug(1, "ProxyPrefixFactory::Constructor");
    if (config.find("proxyPrefix") != config.end())
        ProxyPrefix::ProxyPrefixFile::setProxyPrefix(config.find("proxyPrefix")->second);
    if (config.find("proxyPrefix") != config.end())
        ProxyPrefix::ProxyPrefixFs::setProxyPrefix(config.find("proxyPrefix")->second);
    if (config.size() == 0) {
        std::map<std::string, std::string> defaultconfig;
        log->Debug(1, "config size is zero... This is a \"default plug-in call\" "
                      "-> loading default config file @ XRD_DEFAULT_PLUGIN_CONF "
                      "Environment Variable ");
        loadDefaultConf(defaultconfig);
        // load config for Fileplugin
        if (defaultconfig.find("proxyPrefix") != defaultconfig.end())
            ProxyPrefix::ProxyPrefixFile::setProxyPrefix(defaultconfig.find("proxyPrefix")->second);
        // load config for Filesystemplugin
        if (defaultconfig.find("proxyPrefix") != defaultconfig.end())
            ProxyPrefix::ProxyPrefixFs::setProxyPrefix(defaultconfig.find("proxyPrefix")->second);
    }
    ProxyPrefix::ProxyPrefixFile::printInfo();
}
ProxyPrefixFactory::~ProxyPrefixFactory() {
    XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
    log->Debug(1, "ProxyPrefixFactory::~ProxyPrefixFactory");
}

XrdCl::FilePlugIn* ProxyPrefixFactory::CreateFile(const std::string& url) {
    XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
    log->Debug(1, "ProxyPrefixFactory::CreateFile");
    return static_cast<XrdCl::FilePlugIn*>(new ProxyPrefix::ProxyPrefixFile());
}

XrdCl::FileSystemPlugIn* ProxyPrefixFactory::CreateFileSystem(const std::string& url) {
    XrdCl::Log* log = XrdCl::DefaultEnv::GetLog();
    log->Debug(1, "ProxyPreficFactory::CreateFilesys");
    return static_cast<XrdCl::FileSystemPlugIn*>(new ProxyPrefix::ProxyPrefixFs(url));
}
} // namespace PPFactory
extern "C" {

void* XrdClGetPlugIn(const void* arg) {
    const std::map<std::string, std::string>& pconfig =
      *static_cast<const std::map<std::string, std::string>*>(arg);
    void* plug = new PPFactory::ProxyPrefixFactory(pconfig);
    return plug;
}
}
