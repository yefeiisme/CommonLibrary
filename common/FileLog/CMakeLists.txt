SET(LIBFILELOG_SRC FileLog.cpp)
include_directories (
	"../../Share/Header"
        "../../Share/Header/Base"
)
ADD_LIBRARY(filelog STATIC ${LIBFILELOG_SRC})
INSTALL(TARGETS filelog
        ARCHIVE DESTINATION lib)
