#ifndef __NET_LOG_DEFINE_H__
#define __NET_LOG_DEFINE_H__

#define NET_LOGERR(fmt) \
std::cerr << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << "|" << fmt << "\n" << std::endl;


#endif // __NET_LOG_DEFINE_H__