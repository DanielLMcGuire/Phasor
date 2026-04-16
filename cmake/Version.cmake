# Get latest tag (fallback to 0.0.0 if none exist)
execute_process(
    COMMAND git describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

if(NOT GIT_TAG)
    set(GIT_TAG "0.0.0")
endif()

# Count commits since tag
execute_process(
    COMMAND git rev-list ${GIT_TAG}..HEAD --count
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMITS_SINCE_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get short commit hash
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Detect dirty state (includes untracked files)
execute_process(
    COMMAND git status --porcelain
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_STATUS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(GIT_STATUS STREQUAL "")
    set(GIT_IS_DIRTY FALSE)
else()
    set(GIT_IS_DIRTY TRUE)
endif()

# Build version string
set(GIT_VERSION_STRING "${GIT_TAG}")

if(NOT GIT_COMMITS_SINCE_TAG EQUAL 0)
    set(GIT_VERSION_STRING "${GIT_VERSION_STRING}.${GIT_COMMITS_SINCE_TAG}")
endif()

if(GIT_IS_DIRTY)
    set(GIT_VERSION_STRING "${GIT_VERSION_STRING}-${GIT_COMMIT_HASH}")
endif()