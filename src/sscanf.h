#include <cstdio>
#ifdef _WIN32
inline int sscanf_wrapper(const char* buffer, const char* format, char* str, unsigned int* num) {
    return sscanf_s(buffer, format, str, 64, num);
}
inline int sscanf_wrapper(const char* buffer, const char* format, char* str) {
    return sscanf_s(buffer, format, str, 64);
}
inline int sscanf_wrapper(const char* buffer, const char* format, int* num) {
    return sscanf_s(buffer, format, num);
}
#define sscanf sscanf_wrapper
#endif