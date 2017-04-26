SET(LIBDAEMON_SRC Daemon.cpp)
include_directories (
	"../../Share/Header"
        "../../Share/Header/Base"
)
ADD_LIBRARY(daemon STATIC ${LIBDAEMON_SRC})
INSTALL(TARGETS daemon
        ARCHIVE DESTINATION lib)
