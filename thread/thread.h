#pragma once

#include "thread/mutex.h"
#include "thread/semaphore.h"
#include "util/util.h"
#include <pthread.h>

namespace lon
{
namespace thread
{
class LON_API Thread : public util::Nonecopyable
{
  public:
    using Ptr = std::shared_ptr<Thread>;
    Thread(std::function<void()> cb, const std::string &name = "UNKNOWN");
    virtual ~Thread();

    pid_t getId() const;
    void setId(const pid_t &id);
    std::string getName() const;
    void setName(const std::string &name);
    void join();
    static Thread *getThis();
    static const std::string &getNameStatic();
    static void setNameStatic(const std::string &name);
    static const pid_t getIdStatic();
    static void setIdStatic(const pid_t &id);

  private:
    static void *run(void *arg);

  private:
    pid_t m_id;
    pthread_t m_thread;
    std::function<void()> m_cb;
    std::string m_name;
    Semaphore m_semaphore;
};
} // namespace thread
} // namespace lon