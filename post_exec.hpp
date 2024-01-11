#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <thread>

using namespace std::chrono_literals;
constexpr auto defaultDelay = 1s;

class PostExec {
  public:
    PostExec();
    ~PostExec();

    PostExec(const PostExec& other)            = delete;
    PostExec& operator=(const PostExec& other) = delete;
    PostExec(PostExec&& other)                 = delete;
    PostExec& operator=(PostExec&& other)      = delete;

    void exec(std::function<void()> callback, std::chrono::milliseconds delay);
    void cancel();

  private:
    void startMainLoop();

    std::thread             m_mainLoopThread;
    std::mutex              m_mutex;
    std::condition_variable m_cv;

    std::atomic_bool m_isWaitingCanceled{false};
    std::atomic_bool m_isMainLoopRunning{true};

    std::chrono::milliseconds m_delay{defaultDelay};
    std::function<void()>     m_callback{nullptr};
};