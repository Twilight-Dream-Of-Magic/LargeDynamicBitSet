#ifndef _ENDIANNESS_H_
	#define _ENDIANNESS_H_
	#if defined( __BYTE_ORDER ) && __BYTE_ORDER == __BIG_ENDIAN || defined( __BIG_ENDIAN__ ) || defined( __ARMEB__ ) || defined( __THUMBEB__ ) || defined( __AARCH64EB__ ) || defined( _MIBSEB ) || defined( __MIBSEB ) || defined( __MIBSEB__ )
		#ifndef __BIG_ENDIAN__
			#define __BIG_ENDIAN__
		#endif
	#elif defined( __BYTE_ORDER ) && __BYTE_ORDER == __LITTLE_ENDIAN || defined( __LITTLE_ENDIAN__ ) || defined( __ARMEL__ ) || defined( __THUMBEL__ ) || defined( __AARCH64EL__ ) || defined( _MIPSEL ) || defined( __MIPSEL ) || defined( __MIPSEL__ ) || defined( _WIN32 ) || defined( __i386__ ) || defined( __x86_64__ ) || defined( _X86_ ) || defined( _IA64_ )
		#ifndef __LITTLE_ENDIAN__
			#define __LITTLE_ENDIAN__
		#endif
	#else
		#error "I don't know what architecture this is!"
	#endif
#endif

#ifndef _EXTERN_CONFIG_
	#define _EXTERN_CONFIG_
	#if defined(_MSC_VER)
		#if defined(_DLL)
			#define _EXTERN_EXPORT __declspec(dllexport)
			#define _EXTERN_IMPORT __declspec(dllimport)
		#else
			#define _EXTERN_EXPORT
			#define _EXTERN_IMPORT
		#endif
	#else
		// All modules on Unix are compiled with -fvisibility=hidden
		// All API symbols get visibility default
		// whether or not we're static linking or dynamic linking (with -fPIC)
		#define _EXTERN_EXPORT __attribute__((visibility("default")))
		#define _EXTERN_IMPORT __attribute__((visibility("default")))
	#endif
#endif
#define EXTERN_SYMBOL _EXTERN_IMPORT