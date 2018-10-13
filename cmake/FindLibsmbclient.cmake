#
# Find libsmbclient.h and libsmbclient.so from Samba 4.
#
# This file is in the public domain.
#

# Find the include file
find_path(LIBSMBCLIENT_INCLUDE_DIR NAMES libsmbclient.h PATH_SUFFIXES samba-4.0)

# Find the library
find_library(LIBSMBCLIENT_LIBRARY NAMES smbclient)

# Handle the arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libsmbclient DEFAULT_MSG LIBSMBCLIENT_LIBRARY LIBSMBCLIENT_INCLUDE_DIR)

# Mark as advanced
mark_as_advanced(LIBSMBCLIENT_LIBRARY LIBSMBCLIENT_INCLUDE_DIR)

# Set variables
set(LIBSMBCLIENT_LIBRARIES ${LIBSMBCLIENT_LIBRARY})
set(LIBSMBCLIENT_INCLUDE_DIRS ${LIBSMBCLIENT_INCLUDE_DIR})


