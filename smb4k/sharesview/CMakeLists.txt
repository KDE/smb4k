
include_directories( ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(smb4ksharesview_PART_SRCS
   smb4kshareslistview.cpp
   smb4ksharesiconview.cpp
   smb4ksharesview_part.cpp
   smb4kshareslistviewitem.cpp
   smb4ksharesiconviewitem.cpp
   smb4ksharesviewtooltip.cpp
   smb4ksharesviewitemdata.cpp )

kde4_add_plugin( smb4ksharesview ${smb4ksharesview_PART_SRCS})

target_link_libraries( smb4ksharesview  ${KDE4_KDECORE_LIBS} smb4kdialogs smb4kcore ${KDE4_KPARTS_LIBS} )

install(TARGETS smb4ksharesview DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  smb4ksharesview_part.rc
         DESTINATION ${DATA_INSTALL_DIR}/smb4k )

