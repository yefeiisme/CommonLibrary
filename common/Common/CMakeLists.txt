SET(LIBCOMMON_SRC common.cpp)
include_directories (
	"../../Share/Header"
	"../../Share/Header/Base"
)
ADD_LIBRARY(common STATIC ${LIBCOMMON_SRC})
INSTALL(TARGETS common
        ARCHIVE DESTINATION lib)
