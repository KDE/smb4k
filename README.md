Smb4K README
============
Smb4K is an advanced network neighborhood browser and Samba share mounting utility. It is based on the KDE Frameworks 5, Samba's client library (libsmbclient) and, optionally, the KDSoap WS-Discovery client. It scans your network neighborhood for all available workgroups, servers and shares and can mount all desired shares to your local file system. It is released under the terms of the [GNU General Public License, version 2 (GPL v2+)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

Linux, FreeBSD and its derivatives (e. g. GhostBSD), NetBSD and DragonFly BSD are officially supported.


Features
--------
- Scanning for (active) workgroups, hosts, and shares
- Support of the CIFS and SMB3 file system under Linux® as well as the SMBFS file system under FreeBSD®
- Mounting and unmounting of shares
- Access to the files of a mounted share using a file manager or terminal
- Auto-detection of external mounts and unmounts
- Remounting of previously used shares on program start
- Miscellaneous infos about remote network items and mounted shares
- Network search
- Preview of the contents of a share
- Default login
- Special handling of homes shares
- Ability to bookmark favorite shares
- System tray widget
- Support of advanced mount options
- Support of printer shares
- KWallet support
- Synchronization of a remote share with a local copy and vice versa
- Ability to define custom options for individual servers and shares
- Hardware support through the Solid device integration framework, the KDBusAddons 
  convenience classes and the Qt Network module
- Wake-On-LAN capabilities
- Plasmoid for desktop integration
- Profiles for different network neighborhood setups


Translations
------------
Translations are provided by the KDE translators.


Supported operating systems
---------------------------
Smb4K officially supports Linux, FreeBSD and its derivatives (e. g. GhostBSD), NetBSD and DragonFly BSD.


Requirements
------------
To compile Smb4K, you need:
- CMake (version >= 3.2, https://www.cmake.org)
- GNU Compiler Collection (version >= 5.0, https://gcc.gnu.org)
  or clang/LLVM (version >= 3.0, https://clang.llvm.org)

Smb4K build depends on:
- Qt (version >= 5.14, https://www.qt.io): QtCore, QtGui, QtWidgets, 
  QtTest, QtNetwork, QtPrintSupport, QtQml
- KDE Frameworks 5 (KF5, version >= 5.80.0, https://www.kde.org): ECM, 
  KConfig, KAuth, KDocTools, KIconThemes, KWidgetsAddons, KI18n, 
  KCompletion, KCoreAddons, Solid, KIO, KNotifications, KXmlGui, 
  KJobWidgets, KWallet, KDBusAddons, KConfigWidgets, KWindowSystem,
  KCrash, KDNSSD
- libsmbclient

If you would like to enable WS-Discovery support, you additionally need
(see also below for required build option):
- KDSoap (version >= 1.9.0, 
  https://www.kdab.com/development-resources/qt-tools/kd-soap/)
- KDSoap WS-Discovery client (version >= 0.2, 
  https://github.com/caspermeijn/kdsoap-ws-discovery-client/)

It also runtime depends on:
- KPlasma
- LinuxCIFS utils (Linux only, https://wiki.samba.org/index.php/LinuxCIFS_utils)

To enable full functionality, you may also want to install:
- rsync (https://rsync.samba.org)

The full list of changes can be found in the Git log at 
https://invent.kde.org/network/smb4k. A summary can be found in the ChangeLog file
in the tarball.


Configuration, Compilation and Installation
-------------------------------------------
To configure, compile and install Smb4K follow the steps below. Make sure, you have read the Requirements section before you start.

1. Download the version of Smb4K you are interested in and extract the source tarball:
   ``` bash
   $ tar xvfj smb4k-x.y.z.tar.xz
   ```
2. Replace x.y.z with the version number. Change into the source code directory and create a build directory:
   ``` bash
   $ cd smb4k-x.y.z
   $ mkdir build
   ```
3. Change into the build directory:
   ``` bash
   $ cd build
   ```
4. Configure the source:
   ``` bash
   $ cmake -DCMAKE_INSTALL_PREFIX=`qtpaths --install-prefix` -DCMAKE_BUILD_TYPE=Release ..
     ```
   If Smb4K cannot find some shared libraries after the installation, it might be necessary to add the `-DKDE_INSTALL_PLUGINDIR` argument to the command line above:
   ``` bash
   $ cmake -DCMAKE_INSTALL_PREFIX=`qtpaths --install-prefix` \
     -DKDE_INSTALL_PLUGINDIR=`qtpaths --plugin-dir` -DCMAKE_BUILD_TYPE=Release ..
   ```
   If you want to compile Smb4K with debug symbols, replace `Release` by `Debug`.

   There are some Smb4K specific CMake arguments you might be interested in:
   
   | Argument                                                          | Description                                                         |
   |:--------------------------------------------------------------|:---------------------------------------------------------------|
   | -DSMB4K_INSTALL_PLASMOID=ON/OFF     | Install the plasmoid. This is on by default.   |
   | -DSMB4K_WITH_WS_DISCOVERY=ON/OFF  | Build with WS-Discovery support for browsing. This is off by default. |

5. After the configuration, compile and install Smb4K:
   ``` bash
   $ make && sudo make install
   ```
   If you want to be able to remove Smb4K with your package manager later on, use the following approaches depending on your distribution:
   
     <u>Debian, Ubuntu, openSUSE, Fedora, Redhat, Slackware:</u> Use checkinstall instead of make install. The package should be present in your distribution's repository.
      ``` bash
      $ make && sudo checkinstall
      ```

     <u>Arch, Manjaro:</u> Create a PKGBUILD file inside the source's root directory and run the following command to install Smb4K.
     ``` bash
     $ makepkg -i
     ```
      
   For all other distributions and operating systems, please have a look at the respective documentation.

Debugging the Source Code
-------------------------
If you experience crashes or similar and want to debug the source code yourself, compile the source code with debugging symbols. The procedure is similar to the one described in the section above, except that you need to modify the cmake command slightly:

``` bash
$ cmake -DCMAKE_INSTALL_PREFIX=`qtpaths --install-prefix`  -DCMAKE_BUILD_TYPE=Debug ..
```

If you found the cause for a bug, please let us know. A backtrace or a patch will be much appreciated.


Help and Support
----------------
If you encounter problems when using Smb4K and/or need help or support, please
contact us in our [Help](https://sourceforge.net/p/smb4k/discussion/help/) or [General Discussion](https://sourceforge.net/p/smb4k/discussion/general/) 
forums.

Bugs
----
You are strongly encouraged to commit a bug report to out [bug tracker](https://bugs.kde.org/enter_bug.cgi?product=Smb4k&format=guided), if you find a problem.


Copyright
---------
Smb4K is released under the terms of the GNU General Public License (GPL),
version 2. A copy of the license is available in the file COPYING.


Latest Version
--------------
The latest stable version of Smb4K can always be downloaded from out [project's web page](https://sourceforge.net/projects/smb4k/files/latest/download).

