IF (PYTHON_CONFIG MATCHES "python3" OR USE_SYSTEM_PYTHON MATCHES "3.")
    PYTHON3_MODULE()
    EXPORTS_SCRIPT(catboost3.exports)
ELSE()
    PYTHON2_MODULE()
    EXPORTS_SCRIPT(catboost.exports)
ENDIF()

PYTHON_ADDINCL()

# add only headers for dynamic linking
ADDINCL(
    contrib/python/numpy/numpy/core/include
    contrib/python/numpy/numpy/core/include/numpy
    contrib/python/numpy/numpy/core/src/common
    contrib/python/numpy/numpy/core/src/npymath
)

PEERDIR(
    catboost/libs/train_lib
    catboost/private/libs/algo
    catboost/private/libs/algo_helpers
    catboost/libs/cat_feature
    catboost/libs/data
    catboost/private/libs/data_types
    catboost/private/libs/data_util
    catboost/private/libs/init
    catboost/libs/fstr
    catboost/private/libs/documents_importance
    catboost/libs/eval_result
    catboost/libs/helpers
    catboost/private/libs/hyperparameter_tuning
    catboost/libs/loggers
    catboost/libs/logging
    catboost/libs/metrics
    catboost/libs/model
    catboost/private/libs/options
    catboost/private/libs/quantized_pool_analysis
    catboost/private/libs/target
    library/containers/2d_array
    library/json/writer
)

SRCS(helpers.cpp)

# have to disable them because cython's numpy integration uses deprecated numpy API
NO_COMPILER_WARNINGS()

BUILDWITH_CYTHON_CPP(
    _catboost.pyx
    --module-name _catboost
)

IF (ARCH_AARCH64 OR OS_WINDOWS)
    ALLOCATOR(J)
ELSE()
    ALLOCATOR(LF)
ENDIF()

IF (OS_DARWIN)
    LDFLAGS(-headerpad_max_install_names)
ENDIF()
