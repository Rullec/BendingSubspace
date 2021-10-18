
if(NOT PYTHON3_ROOT_DIR)

set(PYTHON3_ROOT_DIR "D:/Softwares/anaconda3/envs/train")
endif(NOT PYTHON3_ROOT_DIR)


include_directories(${PYTHON3_ROOT_DIR}/include ${PYTHON3_ROOT_DIR}/Lib/site-packages/numpy/core/include)
link_directories(${PYTHON3_ROOT_DIR}/libs)

set(python_libs _tkinter python3 python37)
message(STATUS "set python libs " ${python_libs} "from python root " ${PYTHON3_ROOT_DIR})