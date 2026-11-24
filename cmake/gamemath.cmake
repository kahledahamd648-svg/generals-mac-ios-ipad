# Deterministic simulation math is implemented locally in Core/Libraries/WWVegas/WWMath.
# This applies to every supported platform so Windows and iOS use the same math paths.
set(SAGE_USE_DETERMINISTIC_MATH ON)
add_compile_definitions(USE_DETERMINISTIC_MATH)
message(STATUS "Deterministic simulation math enabled (in-tree implementation)")
