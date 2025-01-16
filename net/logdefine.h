#ifndef __NET_LOG_DEFINE_H__
#define __NET_LOG_DEFINE_H__

#ifdef TESTS
#define NET_LOGERR(fmt) \
std::cerr << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << "|" << fmt << "\n" << std::endl;
#else
#define NET_LOGERR(fmt)
#endif

#ifdef TESTS
#define NET_LOGINFO(fmt) \
std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << "|" << fmt << "\n" << std::endl;
#else
#define NET_LOGINFO(fmt)
#endif

#endif // __NET_LOG_DEFINE_H__