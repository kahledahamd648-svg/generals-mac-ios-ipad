set(GS_OPENSSL FALSE)
set(GAMESPY_SERVER_NAME "server.cnc-online.net")

FetchContent_Declare(
    gamespy
    GIT_REPOSITORY https://github.com/TheAssemblyArmada/GamespySDK.git
    GIT_TAG        07e3d15c500415abc281efb74322ab6d9c857eb8
)

FetchContent_MakeAvailable(gamespy)

# GeneralsX: C&C:Online (Revora) DNS redirection for the GameSpy SDK.
# GSI_DOMAIN_NAME above rewrites "gamespy.com" to "server.cnc-online.net", but some
# SDK-generated names (gpsp.*, natneg1-3.*, <game>.ms<N>.*, <game>.master.*) are not
# published by C&C:Online. Route every SDK DNS lookup through CnCOnline_TranslateHost()
# so they resolve to hosts that exist. Only the GameSpy SDK modules get the resolver
# redefinition; engine LAN / single-player code is not affected.
# Windows is excluded because WinSock declares the resolver functions with WSAAPI.
if(NOT WIN32 AND TARGET gamespy)
    set(GENERALSX_CNCONLINE_DIR "${CMAKE_CURRENT_LIST_DIR}/../Core/GameEngine")

    target_sources(gamespy PRIVATE
        "${GENERALSX_CNCONLINE_DIR}/Source/GameNetwork/GameSpy/CnCOnlineHosts.c"
    )
    target_include_directories(gamespy PRIVATE
        "${GENERALSX_CNCONLINE_DIR}/Include/GameNetwork/GameSpy"
    )
    target_sources(gamespy PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../Core/Libraries/Source/WWVegas/WWLib/cnc_dns_override.c"
    )

    if(TARGET z_generals)
        target_sources(z_generals PRIVATE
            "${GENERALSX_CNCONLINE_DIR}/Source/GameNetwork/GameSpy/CnCOnlineHosts.c"
            "${CMAKE_CURRENT_LIST_DIR}/../Core/Libraries/Source/WWVegas/WWLib/cnc_dns_override.c"
        )
        target_include_directories(z_generals PRIVATE
            "${GENERALSX_CNCONLINE_DIR}/Include/GameNetwork/GameSpy"
        )
        if(APPLE)
            target_link_options(z_generals PRIVATE "-Wl,-u,_CnCOnline_TranslateHost")
        endif()
    endif()

    message(STATUS "GeneralsX: C&C:Online GameSpy DNS redirection enabled")
endif()
