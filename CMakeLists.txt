cmake_minimum_required(VERSION 3.27)

project(vip)

add_definitions(-DMOUSE_EVENT)

set(QT_MIN_VERSION "5.15.10")
set(KF5_MIN_VERSION "5.78.0")

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS 
    Core    # QCommandLineParser, QStringLiteral
    Widgets # QApplication 
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons      # KAboutData
    I18n            # KLocalizedString
    XmlGui          # KXmlGuiWindow
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
    
set(vip_SRCS main.cpp keyboard.cpp key.cpp)

qt5_add_resources(QT_RESOURCES resources.qrc)
add_executable(vip ${vip_SRCS} ${QT_RESOURCES})

target_link_libraries(vip
    Qt5::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::XmlGui
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g0 -O3 -march=native")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_BUILD_TYPE Release)

install(TARGETS vip ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})