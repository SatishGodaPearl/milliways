#ifndef MILLIWAYS_CONFIG_H
#define MILLIWAYS_CONFIG_H

/* Define to 1 if you have the <sys/endian.h> header file. */
#cmakedefine HAVE_ENDIAN_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H 1

/* Define to 1 if <tr1/memory> exists and defines std::tr1::shared_ptr. */
#cmakedefine HAVE_STD_TR1_SHARED_PTR 1

/* Define to 1 if <memory> exists and defines std::shared_ptr. */
#cmakedefine HAVE_STD_SHARED_PTR 1

/* Define to 1 if <boost/shared_ptr.hpp> exists and defines boost::shared_ptr. */
#cmakedefine HAVE_BOOST_SHARED_PTR 1

/* Define to 1 if <functional> exists and defines std::function. */
#cmakedefine HAVE_STD_FUNCTION 1

/* Define to 1 if <tr1/functional> exists and defines std::tr1::function. */
#cmakedefine HAVE_STD_TR1_FUNCTION 1

/* Define to 1 if <boost/function.hpp> exists and defines boost::function. */
#cmakedefine HAVE_BOOST_FUNCTION 1

/* Define to 1 if <functional> exists and defines std::mem_fn. */
#cmakedefine HAVE_STD_MEM_FN 1

/* Define to 1 if <tr1/functional> exists and defines std::tr1::mem_fn. */
#cmakedefine HAVE_STD_TR1_MEM_FN 1

/* Define to 1 if <boost/mem_fn.hpp> exists and defines boost::mem_fn. */
#cmakedefine HAVE_BOOST_MEM_FN 1

/* Define to 1 if <functional> exists and defines std::bind. */
#cmakedefine HAVE_STD_BIND 1

/* Define to 1 if <tr1/functional> exists and defines std::tr1::bind. */
#cmakedefine HAVE_STD_TR1_BIND 1

/* Define to 1 if <boost/bind.hpp> exists and defines boost::bind. */
#cmakedefine HAVE_BOOST_BIND 1

/* Define to 1 if <tr1/tuple> exists and defines std::tr1::tuple. */
#cmakedefine HAVE_STD_TR1_TUPLE 1

/* Define to 1 if <tuple> exists and defines std::tuple. */
#cmakedefine HAVE_STD_TUPLE 1

/* Define to 1 if <boost/tuple/tuple.hpp> exists and defines boost::tuple. */
#cmakedefine HAVE_BOOST_TUPLE 1

/* Define to 1 if <boost/multiprecision/cpp_dec_float.hpp> exists and defines boost::multiprecision::cpp_dec_float. */
#cmakedefine HAVE_BOOST_DEC_FLOAT 1

#define SIZEOF_SHORT @SIZEOF_SHORT@
#define SIZEOF_INT @SIZEOF_INT@
#define SIZEOF_LONG @SIZEOF_LONG@
#define SIZEOF_UNSIGNED_LONG @SIZEOF_UNSIGNED_LONG@
#define SIZEOF_LONG_LONG @SIZEOF_LONG_LONG@
#define SIZEOF_FLOAT @SIZEOF_FLOAT@
#define SIZEOF_DOUBLE @SIZEOF_DOUBLE@
#define SIZEOF_SIZE_T @SIZEOF_SIZE_T@
#define SIZEOF_SSIZE_T @SIZEOF_SSIZE_T@

/* Define to 1 if an explicit template for size_t is not needed if all the uint*_t types are there */
#cmakedefine DOESNT_NEED_TEMPLATED_SIZE_T 1

/* Define to 1 if an explicit template for size_t is allowed even if all the uint*_t types are there */
#cmakedefine ALLOWS_TEMPLATED_SIZE_T 1

#endif /* MILLIWAYS_CONFIG_H */
