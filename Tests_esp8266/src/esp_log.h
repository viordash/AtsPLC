
#define ESP_LOGE(tag, format, ...) printf("E| %s |" format, tag, ##__VA_ARGS__)
#define ESP_LOGW(tag, format, ...) printf("W| %s |" format, tag, ##__VA_ARGS__)
#define ESP_LOGI(tag, format, ...) printf("I| %s |" format, tag, ##__VA_ARGS__)
#define ESP_LOGD(tag, format, ...) printf("D| %s |" format, tag, ##__VA_ARGS__)
#define ESP_LOGV(tag, format, ...) printf("V| %s |" format, tag, ##__VA_ARGS__)