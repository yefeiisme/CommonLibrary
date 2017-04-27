SET(LIBRINGBUFFER_SRC
RingBuffer.cpp
)
include_directories (
	"../../Share/Header"
        "../../Share/Header/Base"
)
ADD_LIBRARY(ringbuffer STATIC ${LIBRINGBUFFER_SRC})
INSTALL(TARGETS ringbuffer
        ARCHIVE DESTINATION lib)
