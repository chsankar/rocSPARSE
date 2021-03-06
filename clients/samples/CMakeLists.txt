# ########################################################################
# Copyright (c) 2019 Advanced Micro Devices, Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# ########################################################################

set(ROCSPARSE_CLIENTS_COMMON ../common/utility.cpp)

# Function to add rocsparse examples
function(add_rocsparse_example EXAMPLE_SOURCE)
  get_filename_component(EXAMPLE_TARGET ${EXAMPLE_SOURCE} NAME_WE)
  add_executable(${EXAMPLE_TARGET} ${EXAMPLE_SOURCE} ${ROCSPARSE_CLIENTS_COMMON})

  # rocSPARSE headers
  target_include_directories(${EXAMPLE_TARGET}
    PRIVATE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  )

  # rocSPARSE library
  if(NOT TARGET rocsparse)
    target_link_libraries(${EXAMPLE_TARGET} PRIVATE ${ROCSPARSE_LIBRARIES})
  else()
    target_link_libraries(${EXAMPLE_TARGET} PRIVATE roc::rocsparse)
  endif()

  target_link_libraries(${EXAMPLE_TARGET} PRIVATE hip::host)

  # Add amdgpu targets
  if(CMAKE_CXX_COMPILER MATCHES ".*/hcc$")
    foreach(target ${AMDGPU_TARGETS})
      target_link_libraries(${EXAMPLE_TARGET} PRIVATE --amdgpu-target=${target})
    endforeach()
  endif()

  set_target_properties(${EXAMPLE_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/staging")
endfunction()

# Examples

# Auxiliary
add_rocsparse_example(example_handle.cpp)
add_rocsparse_example(example_fortran_auxiliary.f90)

# Level 1
add_rocsparse_example(example_axpyi.cpp)
add_rocsparse_example(example_doti.cpp)
add_rocsparse_example(example_dotci.cpp)
add_rocsparse_example(example_fortran_dotci.f90)
add_rocsparse_example(example_gthr.cpp)
add_rocsparse_example(example_gthrz.cpp)
add_rocsparse_example(example_roti.cpp)
add_rocsparse_example(example_fortran_roti.f90)
add_rocsparse_example(example_sctr.cpp)

# Level 2
add_rocsparse_example(example_bsrmv.cpp)
add_rocsparse_example(example_bsrsv.cpp)
add_rocsparse_example(example_coomv.cpp)
add_rocsparse_example(example_csrmv.cpp)
add_rocsparse_example(example_ellmv.cpp)
add_rocsparse_example(example_hybmv.cpp)
add_rocsparse_example(example_csrsv.cpp)
add_rocsparse_example(example_fortran_bsrsv.f90)
add_rocsparse_example(example_fortran_csrsv.f90)
add_rocsparse_example(example_fortran_spmv.f90)

# Level 3
add_rocsparse_example(example_csrmm.cpp)
add_rocsparse_example(example_csrsm.cpp)
add_rocsparse_example(example_fortran_csrsm.f90)

# Extra
add_rocsparse_example(example_csrgeam.cpp)
add_rocsparse_example(example_csrgemm.cpp)

# Preconditioner
add_rocsparse_example(example_fortran_csric0.f90)
add_rocsparse_example(example_fortran_csrilu0.f90)
