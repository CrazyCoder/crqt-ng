
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
  set(SRC_LIST ${SRC_LIST} crqt.rc)
  set_property(SOURCE crqt.rc PROPERTY OBJECT_DEPENDS ${CMAKE_BINARY_DIR}/config.h)
endif(WIN32)

set(LANGUAGES ru uk cs bg hu)
message( "Languages: ${LANGUAGES}" )
set(LANGUAGE_TS_FILES)
foreach(LANGUAGE ${LANGUAGES})
  set(TS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/i18n/crqt_${LANGUAGE}.ts")
  list(APPEND LANGUAGE_TS_FILES ${TS_FILE})
endforeach(LANGUAGE ${LANGUAGES})

qt5_add_resources(CRQT_RCS crqtres.qrc)

set(QM_FILES)
if(LANGUAGE_TS_FILES)
  qt5_update_translations(QM_FILES ${LANGUAGE_TS_FILES} ${SRC_LIST})
endif(LANGUAGE_TS_FILES)

if (WIN32)
  # -mwindows -> window application (without terminal screen)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
  add_executable(crqt WIN32 ${SRC_LIST} ${CRQT_RCS} ${QM_FILES} ${RES_FILES})
else()
  add_executable(crqt ${SRC_LIST} ${CRQT_RCS} ${QM_FILES})
endif (WIN32)

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
  if (CMAKE_HOST_UNIX)
    configure_file(valgrind_check.sh.cmake ${CMAKE_CURRENT_BINARY_DIR}/valgrind_check.sh)
  endif(CMAKE_HOST_UNIX)
endif(${CMAKE_BUILD_TYPE} STREQUAL Debug)

if(WIN32)
if(CMAKE_GENERATOR MATCHES "Visual Studio.*")
  message("Visual Studio generator detected")
  set_target_properties(crqt PROPERTIES
      LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
  set_target_properties(crqt PROPERTIES
      LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
  set_target_properties(crqt PROPERTIES
      LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
  set_target_properties(crqt PROPERTIES
      LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
endif(CMAKE_GENERATOR MATCHES "Visual Studio.*")
endif(WIN32)

target_link_libraries(crqt crengine-ng::crengine-ng Qt5::Core Qt5::Gui Qt5::Widgets)

if (MACOS)
  install( TARGETS crqt RUNTIME DESTINATION Contents/MacOS )
  #install( DIRECTORY ../data/skins DESTINATION share/crqt/skins )
  install( DIRECTORY ../data/textures DESTINATION Contents/Resources )
  install( DIRECTORY ../data/backgrounds DESTINATION Contents/Resources )
  install( FILES ${QM_FILES} DESTINATION Contents/Resources/i18n )
  install( FILES desktop/crqt.icns DESTINATION Contents/Resources )
  configure_file(desktop/Info.plist.cmake ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
  install( FILES ${CMAKE_CURRENT_BINARY_DIR}/Info.plist DESTINATION Contents )
  install( FILES desktop/PkgInfo DESTINATION Contents )
  # install crengine-ng data files
  install( DIRECTORY ${CRENGINE_NG_DATA_DIR}/ DESTINATION Contents/Resources FILES_MATCHING PATTERN "*.css" )
  install( DIRECTORY ${CRENGINE_NG_DATA_DIR}/hyph/ DESTINATION Contents/Resources/hyph FILES_MATCHING PATTERN "*.pattern" )
elseif (UNIX)
  install( TARGETS crqt RUNTIME DESTINATION bin )
  #install( DIRECTORY ../data/skins DESTINATION share/crqt/skins )
  install( DIRECTORY ../data/textures DESTINATION share/crqt )
  install( DIRECTORY ../data/backgrounds DESTINATION share/crqt )
  install( FILES ${QM_FILES} DESTINATION share/crqt/i18n )
  install( FILES desktop/crqt.desktop DESTINATION share/applications )
  install( FILES desktop/crqt.appdata.xml DESTINATION share/metainfo )
  install( FILES desktop/crqt.png DESTINATION share/pixmaps )
  install( FILES desktop/crqt.xpm DESTINATION share/pixmaps )
else()
  install( TARGETS crqt RUNTIME DESTINATION . )
  #install( DIRECTORY ../data/skins DESTINATION . )
  install( DIRECTORY ../data/textures DESTINATION . )
  install( DIRECTORY ../data/backgrounds DESTINATION . )
  install( FILES ${QM_FILES} DESTINATION i18n )
endif()
