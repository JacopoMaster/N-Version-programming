find_package(Threads REQUIRED)
add_executable(input_manager input_manager.c)
target_compile_options(input_manager PRIVATE -Wall -g -Wextra -Werror -Wshadow)

add_executable(failure_manager failure_manager.c)
target_compile_options(failure_manager PRIVATE -Wall -g -Wextra -Werror -Wshadow)

add_executable(decision_function decision_function.c)
target_compile_options(decision_function PRIVATE -Wall -g -Wextra -Werror -Wshadow)
target_link_libraries( decision_function PRIVATE Threads::Threads )

add_executable(watchdog watchdog.c)
target_compile_options(watchdog PRIVATE -Wall -g -Wextra -Wshadow)

add_executable(P1 P1.c)
target_compile_options(P1 PRIVATE -Wall -g -Wextra -Werror -Wshadow)

add_executable(P2 P2.c)
target_compile_options(P2 PRIVATE -Wall -g -Wextra -Werror -Wshadow)

add_executable(P3 P3.c)
target_compile_options(P3 PRIVATE -Wall -g -Wextra -Werror -std=c99 -Wshadow)