
target_sources (main-app_lib PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/Manager.cc
	${CMAKE_CURRENT_SOURCE_DIR}/Launcher.cc
	${CMAKE_CURRENT_SOURCE_DIR}/Model.cc
	${CMAKE_CURRENT_SOURCE_DIR}/Simulation.cc
	)

target_include_directories (main-app_lib PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}"
	)
