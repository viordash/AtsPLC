#define LOG_LEVEL_ALL 0
#define LOG_LEVEL_VERBOSE 1
#define LOG_LEVEL_DEBUG 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_WARNING 4
#define LOG_LEVEL_ERROR 5
#define LOG_LEVEL_NONE 6

#define MIN_LOG_LEVEL LOG_LEVEL_ERROR

#define ESP_LOGE(tag, format, ...)                                                                 \
    {                                                                                              \
        if (MIN_LOG_LEVEL <= LOG_LEVEL_ERROR)                                                       \
            printf("E| %s |" format "\n", tag, ##__VA_ARGS__);                                     \
    }
#define ESP_LOGW(tag, format, ...) {if (MIN_LOG_LEVEL <= LOG_LEVEL_WARNING) printf("W| %s |" format "\n", tag, ##__VA_ARGS__);}
#define ESP_LOGI(tag, format, ...) {if (MIN_LOG_LEVEL <= LOG_LEVEL_INFO) printf("I| %s |" format "\n", tag, ##__VA_ARGS__);}
#define ESP_LOGD(tag, format, ...) {if (MIN_LOG_LEVEL <= LOG_LEVEL_DEBUG) printf("D| %s |" format "\n", tag, ##__VA_ARGS__);}
#define ESP_LOGV(tag, format, ...) {if (MIN_LOG_LEVEL <= LOG_LEVEL_VERBOSE) printf("V| %s |" format "\n", tag, ##__VA_ARGS__);}