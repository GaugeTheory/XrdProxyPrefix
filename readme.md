#XrdOpenLocal 
An XRootD client plug-in to "redirect" XRootD calls to a locally mounted file system ( and an option to tunneling through an XRootD - proxy  if configured, because we cannot chain client plug-ins easily...).
This plug-in is considered in early development, as many methods have not yet been properly implemented. It does however work with xrdcp and simple tasks with ROOT's TNetXNGFile. 

In the future, we want to use this plug-in to access data that is provided via a Lustre file system, improving I/O performance by bypassing the need to read indirectly via local XRootD data servers.

# Warning
The source tree includes private header files of the XrdCl source tree and needs to be kept in sync with the XRootD version source used!

# Plug-in configuration

This plug-in is configured via a plug-in configuration file in /etc/xrootd/client.plugins.d/.conf, or in the users ~/.xrootd.client.plugins.d/.conf

Example configuration to delegate all I/O to dataserver.test to this plug-in:
```shell
url = root://dataserver.test;root://dataserver2.test
lib = /installdir/XrdOpenLocal/XrdOpenLocal.so
redirectlocal = dataserver.test|/tmp/d1;dataserver2.test|/tmp/d2      
enable = true
```
## Configuring the target-location binding

"redirectlocal" shows to a point in the file system where you want to "redirect" your calls to.
'|' delimits the server from that point, multiple combinations can be delimited with ';'.
In the example above, "root://dataserver.test:1094//foo/bar" would be changed to "/tmp/d1/foo/bar" .

## As default plug-in

Additionally, XRootD allows to set a client plug-in as default to use via the XRD_PLUGIN environmental variable.
Sadly I have yet to find a vanilla way to give a configuration file to the default plug-in.
In the meantime, you need to set the XrdRedirLocDEFAULTCONF environmental variable to the specific config file, if you want to use it as a default plug-in.

# Install and tests
To compile the plug-in, you need to set the XRD_PATH environmental variable to the toplevel of your XRootD installation.

You can compile the plug-in library with :
```shell
make
```
You can run two simple bash-tests using xrdcp with :
```shell
make test
```
# Usage
When using this plug-in, all high level XRootD calls (xrdcp, from TNetXNGFile in ROOT, etc.) to targets configured in the config file, should instead be "redirected" to a file available in the local file system.
Have a look at the tests, if you want to know how to use this plug-in.

# License
The XrdOpenLocal plug-in is distributed under the terms of the GNU Lesser Public Licence version 3 (LGPLv3)

# ToDo's
* Check if the file is actually available locally, fall back to the base implementation if not.
* Check permissions, try to contact a data server once and check if the user/token/... has access to a requested file
* Implement missing file-/file system- methods



