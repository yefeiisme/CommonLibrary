SET(LIBFILEREADER_SRC
KDebug.cpp
KFile.cpp
KFilePath.cpp
KIniFile.cpp
KLinkArray.cpp
KMemBase.cpp
KMemClass.cpp
KMemStack.cpp
KPakFile.cpp
KStrBase.cpp
KTabFile.cpp
)
include_directories (
	"../../Share/Header"
	"../../Share/Header/Base"
)
ADD_LIBRARY(filereader STATIC ${LIBFILEREADER_SRC})
INSTALL(TARGETS filereader
        ARCHIVE DESTINATION lib)
