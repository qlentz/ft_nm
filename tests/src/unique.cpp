// GNU unique symbols ('u') and C++ mangled names.
inline int	counter() { static int c; return ++c; }
template <class T> struct S { static int v; };
template <class T> int S<T>::v = 3;
namespace ns { int very_long_function_name_to_mangle(int, double, const char *) { return 0; } }
int	use() { return counter() + S<int>::v + S<double>::v; }
