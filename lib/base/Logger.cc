/**
* FileName:   Log.cc
* CreateDate: 2022-03-03 23:08:32
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#include "Logger.h"

using namespace SinBack;

// 静态变量初始化
std::unordered_map<Log::Logger::string_type, Log::Logger*> Log::Logger::logger_map;
std::mutex Log::Logger::logger_mutex;

Log::Logger::Logger(Log::LoggerType type, const Log::Logger::string_type &file_name, Size_t thread_num)
        : datetime_()
        , th_num_(thread_num)
        , level_(Debug)
        , stop_(false)
        , time_str_{0}
        , sec_(0)
        , type_(type)
        , min_(0)
{
    this->front_buf_.reset(new queue_type);
    this->back_buf_.reset(new queue_type);
    Base::getDateTimeNow(&this->datetime_);
    formatTime();
    this->sec_ = datetime_.sec;

    string_type name = file_name;
    if (name.empty()){
        name = "SinBack_Logger";
    }
    if (type_ == Normal){
        this->log_.reset(new Base::LogFile(name.c_str()));
    }else if (type_ == Rolling){
        this->log_.reset(new Base::RollLogFile(name.c_str()));
    }
    if (this->th_num_ == 0){
        this->th_num_ = 1;
    }
    Size_t i = 0;
    for (; i < this->th_num_; ++i){
        this->ths_.emplace_back(&Logger::thread_run_func, this);
    }
}

/**
 * @brief 线程运行函数
 */
void Log::Logger::thread_run_func()
{
    string_type buf;
    while (true){
        {
            {
                std::unique_lock<std::mutex> lock(this->back_mutex_);
                // 如果后端缓冲区为空并且还在运行，则休眠超时 3s
                if (this->back_buf_->empty() && !this->stop_) {
                    this->cv_.wait_for(lock, std::chrono::seconds(3), [this] {
                        return (this->stop_) || (!this->back_buf_->empty() ||
                            this->front_buf_->size() >= front_queue_max_size);
                    });
                }
                // 如果超时唤醒并且后端队列为空，则交换缓冲区
                {
                    std::unique_lock<std::mutex> lock1(this->front_mutex_);
                    if (this->back_buf_->empty()) {
                        this->back_buf_.swap(this->front_buf_);
                    }
                }
                // 全部数据都处理完成，则退出
                if (this->stop_ && this->back_buf_->empty()) {
                    break;
                }
                // 写入文件
                if (!this->back_buf_->empty()) {
                    buf = std::move(this->back_buf_->front());
                    this->back_buf_->pop();
                    if (this->type_ == Normal) {
                        this->log_->write(buf);
                    } else if (this->type_ == Rolling) {
                        std::dynamic_pointer_cast<Base::RollLogFile>(this->log_)->write(buf);
                    }
                }
            }
            // 通知其他线程
            this->cv_.notify_one();
        }
    }
}

/**
 * @brief 格式化时间
 */
void Log::Logger::formatTime()
{
    snprintf(time_str_, 20, "%04d-%02d-%02d %02d:%02d:%02d",
             datetime_.year, datetime_.month, datetime_.day, datetime_.hour, datetime_.min, datetime_.sec);
}

/**
 * @brief 格式化时间秒部分
 */
void Log::Logger::formatTimeSec()
{
    snprintf(time_str_ + 17, 3, "%02d", this->sec_);
}

Log::Logger::~Logger(){
    {
        std::unique_lock<std::mutex> lock(this->back_mutex_);
        this->stop_ = true;
    }
    // 通知其他线程结束
    this->cv_.notify_all();
    // 等待线程结束
    for (auto& it : this->ths_){
        if (it.joinable()){
            it.join();
        }
    }
}

/**
 * @brief 获取日志记录器
 * @param name : 日志记录器名称
 * @return : 有返回日志记录器，没有返回nullptr
 */
Log::Logger* Log::Logger::getLogger(const Log::Logger::string_type &name)
{
    std::unique_lock<std::mutex> lock(logger_mutex);
    auto it = logger_map.find(name);
    if (it != logger_map.end()){
        return it->second;
    }
    return nullptr;
}

/**
 * @brief 注册日志记录器
 * @param name : 日志记录器名称
 * @param type : 日志记录器类型
 * @return : 是否注册成功
 */
bool Log::Logger::registerLogger(const Log::Logger::string_type &name, const Logger::string_type& log_path,
                                 LoggerType type, LogLevel level, Size_t thread_num)
{
    if (log_path.empty() || name.empty()){
        return false;
    }
    std::unique_lock<std::mutex> lock(logger_mutex);
    auto it = logger_map.find(name);
    if (it != logger_map.end()){
        return false;
    }
    auto* logger = new Logger(type, log_path, thread_num);
    logger->setLogLevel(level);
    logger_map.insert(std::make_pair(name, logger));
    return true;
}

/**
 * @brief 取消注册日志记录器
 * @param name : 日志记录器名称
 * @return : 是否注销成功
 */
bool Log::Logger::unregisterLogger(const Log::Logger::string_type &name)
{
    std::unique_lock<std::mutex> lock(logger_mutex);
    auto it = logger_map.find(name);
    if (it == logger_map.end()){
        return false;
    }
    delete it->second;
    logger_map.erase(it);
    return true;
}

/**
 * @brief 取消注册所有日志记录器
 */
void Log::Logger::unregisterAllLogger()
{
    std::unique_lock<std::mutex> lock(logger_mutex);
    for (auto& it : logger_map){
        delete it.second;
    }
    logger_map.clear();
}
