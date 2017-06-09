SET(LIBNETWORK_SRC
conn_info/conn_info.cpp
Server/server_network_epoll.cpp
Client/client_network_select.cpp
)
include_directories (
	"../../Share/Header"
        "../../Share/Header/Base"
)
ADD_LIBRARY(network STATIC ${LIBNETWORK_SRC})
INSTALL(TARGETS network
        ARCHIVE DESTINATION lib)
