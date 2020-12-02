#XrdProxyPrefix
An XRootD client plug-in to forward XRootD calls over an XrootD DIRECT MODE proxy to its destination.
This plug-in is considered in early development, as many methods have not yet been properly implemented. It does however work with xrdcp and simple tasks with ROOT's TNetXNGFile and will be tested in an ALICE environment. 
Right now a full File and Filesystem implementation is provided but not tested thoroughly.
(If you find any bugs - feel free to open an issue!)

## Warning
The sources include private header files of the XrdCl source tree and needs to be kept in sync with the XRootD version source used!

## Plug-in configuration

This plug-in should be configured by using it as an Default Plugin. (Setting the XRD_PLUGIN environmental variable) It can then be configured by a configuration file specified in the XRD_DEFAULT_PLUGIN_CONF environmental variable.
Sadly there is no vanilla way to give a configuration file to a default plug-in.

Example configuration to delegate all I/O over a forwarding proxy:
```shell
lib = /installdir/XrdProxy/XrdProxyPrefix.so
proxyPrefix=://myProxyHostName/
enable = true
```
## Configuring the target-location binding

"Proxy" shows to the DIRECT MODE proxy you want to tunnel your connections through a point in the file system where you want to "redirect" your calls to.
In this example a call like
"root://dataserver.test:1094//foo/bar" 
would be changed to 
"root://myProxyHostName//foo/bar" 
to be forwarded through the proxy where has "pss.origin dataserver.test:1094".

## Install
To compile the plug-in, you need to set the XRD_PATH environmental variable to the top level of your XRootD installation.

You can compile the plug-in library with :
```shell
make
```
# Usage
When using this plug-in, all high level XRootD calls (xrdcp, from TNetXNGFile in ROOT, etc.) should be forwarded over the forward proxy(or proxy manager) configured in the configuration file.


# License
The XrdProxyPrefix plug-in is distributed under the terms of the GNU Lesser Public Licence version 3 (LGPLv3)




