cmake_minimum_required(VERSION 3.16)

project(HotmailBotQt VERSION 0.4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

set(HOTMAIL_BOT_SOURCES
    src/main.cpp
    src/ui/MainWindow.cpp
    src/core/TaskManager.cpp
    src/core/AccountWorker.cpp
    src/core/AccountProfile.cpp
    src/core/RegistrationPipeline.cpp
    src/core/RegistrationResult.cpp
    src/core/ConfigStore.cpp
    src/core/ResultStore.cpp
    src/net/ProxyManager.cpp
    src/bridge/AutomationClient.cpp
    src/bridge/MockBrowserDriver.cpp
    src/bridge/WebDriverClient.cpp
    src/bridge/WebDriverBrowserDriver.cpp
    src/bridge/BrowserDriverFactory.cpp
    src/core/EmailVerificationService.cpp
    src/core/SignupSelectors.cpp
    src/core/RetryPolicy.cpp
    src/core/DiagnosticsStore.cpp
    src/bridge/SkipCaptchaSolver.cpp
    src/bridge/ManualCaptchaSolver.cpp
    src/bridge/HttpCaptchaSolver.cpp
    src/bridge/CaptchaSolverFactory.cpp
    src/core/SelectorConfig.cpp
    src/core/PageState.cpp
    src/core/PageStateDetector.cpp
    src/core/SignupFlowRunner.cpp
    src/core/TempMailClient.cpp
    src/core/ConfigSyncService.cpp
    resources/resources.qrc
)

add_executable(${PROJECT_NAME} ${HOTMAIL_BOT_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
    Qt6::Network
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
