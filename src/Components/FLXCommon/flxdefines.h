#ifndef FLXDEFINES_HEADER
#define FLXDEFINES_HEADER

/*unix*/
#ifdef __unix__
#ifndef unix
#define unix
#endif
#endif

/*linux*/
#ifdef __linux__
#ifndef linux
#define linux
#endif
#endif

#ifdef linux
#define FLXLINUX
#endif


#ifdef unix
#define FLXUNIX
#endif

// define WIN
#if defined(_WIN32) || defined(_WIN64)//windows
#define FLXWIN
#endif

#endif
