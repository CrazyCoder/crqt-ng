
# Find includes in corresponding current build & source directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(SRC_LIST
  aboutdlg.ui
  aboutdlg.cpp
  bookmarklistdlg.ui
  bookmarklistdlg.cpp
  crqtutil.cpp
  main.cpp
  recentdlg.ui
  recentdlg.cpp
  tocdlg.ui
  tocdlg.cpp
  addbookmarkdlg.ui
  addbookmarkdlg.cpp
  cr3widget.cpp
  filepropsdlg.ui
  filepropsdlg.cpp
  mainwindow.ui
  mainwindow.cpp
  settings.ui
  settings.cpp
  wolexportdlg.ui
  wolexportdlg.cpp
  exportprogressdlg.ui
  exportprogressdlg.cpp
  searchdlg.ui
  searchdlg.cpp
  fallbackfontsdialog.ui
  fallbackfontsdialog.cpp
)

if(WIN32)
  set(SRC_LIST ${SRC_LIST} cr3.rc)
  set_property(SOURCE cr3.rc PROPERTY OBJECT_DEPENDS ${CMAKE_BINARY_DIR}/config.h)
endif(WIN32)

if(UNIX)
  add_definitions(-DCRUI_DATA_DIR="${CMAKE_INSTALL_PREFIX}/share/cruiqt/")
else()
  add_definitions(-DCRUI_DATA_DIR="")
endif()

set(LANGUAGES ru uk cs de es pl bg hu)
message( "Languages: ${LANGUAGES}" )
set(LANGUAGE_TS_FILES)
foreach(LANGUAGE ${LANGUAGES})
  set(TS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/i18n/cr3_${LANGUAGE}.ts")
  list(APPEND LANGUAGE_TS_FILES ${TS_FILE})
  #set_source_files_properties(${TS_FILE} PROPERTIES OUTPUT_LOCATION
  #      "${CMAKE_CURRENT_SOURCE_DIR}/i18n")
endforeach(LANGUAGE ${LANGUAGES})

qt5_add_resources(CR3_RCS cr3res.qrc)

set(QM_FILES)
if(LANGUAGE_TS_FILES)
  qt5_update_translations(QM_FILES ${LANGUAGE_TS_FILES} ${SRC_LIST})
endif(LANGUAGE_TS_FILES)

if (UNIX)
  find_program(GZIP_TOOL
    NAMES gzip
    PATHS /bin
      /usr/bin
      /usr/local/bin
  )
  if(NOT GZIP_TOOL)
    message(FATAL_ERROR "Unable to find 'gzip' program")
  endif(NOT GZIP_TOOL)

  set (man ${CMAKE_SOURCE_DIR}/data/docs/cr3.1)
  set (man_gz ${CMAKE_CURRENT_BINARY_DIR}/cr3.1.gz)

  add_custom_command(OUTPUT ${man_gz}
    COMMAND ${GZIP_TOOL} -9 -c ${man} > ${man_gz}
    DEPENDS ${man}
    COMMENT "Building ${man_gz}")

  set (CR3_MAN_PAGES ${man_gz})
else()
  set (CR3_MAN_PAGES)
endif(UNIX)

if (WIN32)
  # -mwindows -> window application (without terminal screen)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
  add_executable(cruiqt WIN32 ${CR3_MAN_PAGES} ${SRC_LIST} ${CR3_RCS} ${QM_FILES} ${RES_FILES})
else()
  add_executable(cruiqt ${CR3_MAN_PAGES} ${SRC_LIST} ${CR3_RCS} ${QM_FILES})
endif (WIN32)

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
  if (CMAKE_HOST_UNIX)
    configure_file(valgrind_check.sh.cmake ${CMAKE_CURRENT_BINARY_DIR}/valgrind_check.sh)
  endif(CMAKE_HOST_UNIX)
endif(${CMAKE_BUILD_TYPE} STREQUAL Debug)

if(WIN32)
if(CMAKE_GENERATOR MATCHES "Visual Studio.*")
  message("Visual Studio generator detected")
  set_target_properties(cruiqt PROPERTIES
      LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
  set_target_properties(cruiqt PROPERTIES
      LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
  set_target_properties(cruiqt PROPERTIES
      LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
  set_target_properties(cruiqt PROPERTIES
      LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
endif(CMAKE_GENERATOR MATCHES "Visual Studio.*")
endif(WIN32)


target_link_libraries(cruiqt crengine-ng::crengine-ng Qt5::Core Qt5::Gui Qt5::Widgets)

if (MAC)
  install( TARGETS cruiqt RUNTIME DESTINATION Contents/MacOS )
  #install( DIRECTORY ../data/skins DESTINATION share/cruiqt/skins )
  install( DIRECTORY ../data/textures DESTINATION Contents/Resources )
  install( DIRECTORY ../data/backgrounds DESTINATION Contents/Resources )
  #install( FILES ${CR3_MAN_PAGES} DESTINATION share/doc/cruiqt )
  install( FILES ${QM_FILES} DESTINATION Contents/Resources/i18n )
  install( DIRECTORY ${QT_INCLUDE_DIR}/../src/gui/mac/qt_menu.nib DESTINATION Contents/Resources )
  install( FILES desktop/Info.plist DESTINATION Contents )
  install( FILES desktop/PkgInfo DESTINATION Contents )
  #install( FILES desktop/cr3.png DESTINATION share/pixmaps )
  #install( FILES desktop/cr3.xpm DESTINATION share/pixmaps )
elseif (UNIX)
  install(FILES ${man_gz}
    DESTINATION "share/man/man1"
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
  install( TARGETS cruiqt RUNTIME DESTINATION bin )
  #install( DIRECTORY ../data/skins DESTINATION share/cruiqt/skins )
  install( DIRECTORY ../data/textures DESTINATION share/cruiqt )
  install( DIRECTORY ../data/backgrounds DESTINATION share/cruiqt )
  #install( FILES ${CR3_MAN_PAGES} DESTINATION share/doc/cruiqt )
  install( FILES ${QM_FILES} DESTINATION share/cruiqt/i18n )
  install( FILES desktop/cr3.desktop DESTINATION share/applications )
  install( FILES desktop/cr3.appdata.xml DESTINATION share/metainfo )
  install( FILES desktop/cr3.png DESTINATION share/pixmaps )
  install( FILES desktop/cr3.xpm DESTINATION share/pixmaps )
else()
  install( TARGETS cruiqt RUNTIME DESTINATION . )
  #install( DIRECTORY ../data/skins DESTINATION . )
  install( DIRECTORY ../data/textures DESTINATION . )
  install( DIRECTORY ../data/backgrounds DESTINATION . )
  install( FILES ${QM_FILES} DESTINATION i18n )
endif()
