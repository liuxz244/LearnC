#ifndef __dbg_h__  // 防止重复包含该头文件的宏定义
#define __dbg_h__

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef NDEBUG  // 如果定义了NDEBUG：
#define debug(M, ...)  // 则debug宏不做任何事
#else  // 否则，输出调试信息到标准错误流，包含源文件名、行号以及自定义消息
#define debug(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

// 如果 errno 为 0，返回 "None"，否则返回 errno 对应的错误描述字符串
#define clean_errno() (errno == 0 ? "None" : strerror(errno))

// 输出错误日志，包含文件名、行号、函数名、errno 信息和自定义消息
#define log_err(M, ...) fprintf(stderr, "[ERROR] (%s:%d:%s(): errno: %s) " M "\n", __FILE__, __LINE__, __func__, clean_errno(), ##__VA_ARGS__)

// 输出警告日志，格式和 log_err 类似，但标签为 [WARN]
#define log_warn(M, ...) fprintf(stderr, "[WARN] (%s:%d:%s(): errno: %s) " M "\n", __FILE__, __LINE__, __func__, clean_errno(), ##__VA_ARGS__)

// 输出普通信息日志，包含文件名、行号、函数名
#define log_info(M, ...) fprintf(stderr, "[INFO] (%s:%d:%s()) " M "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)


// 检查条件 A 是否成立，如果不成立：
//   1. 输出错误日志
//   2. 将 errno 置 0（防止误用之前的 errno 值）
//   3. 跳转到标签 error（用于错误处理的统一出口）
#define check(A, M, ...) if(!(A)) { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

// 强制进入错误处理（无条件触发），输出错误日志并跳到 error
#define sentinel(M, ...)  { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

// 专门用于检查内存分配是否成功的宏（实现上就是调用 check）
#define check_mem(A) check((A), "Out of memory.")

// 调试版的 check，如果条件不成立，输出调试信息而不是错误信息
#define check_debug(A, M, ...) if(!(A)) { debug(M, ##__VA_ARGS__); errno=0; goto error; }

#endif
