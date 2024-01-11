#include "post_exec.hpp"

using namespace std::chrono_literals;

PostExec::PostExec() {
    m_mainLoopThread = std::thread(&PostExec::startMainLoop, this);
}

PostExec::~PostExec() {
    m_isMainLoopRunning = false;
    cancel();
    m_mainLoopThread.join();
}

void PostExec::exec(std::function<void()>     callback,
                    std::chrono::milliseconds delay) {
    {
        std::lock_guard<std::mutex> lck(m_mutex);

        m_callback          = callback;
        m_delay             = delay;
        m_isWaitingCanceled = true;
    }
    m_cv.notify_one();
}

void PostExec::cancel() {
    {
        std::lock_guard<std::mutex> lck(m_mutex);

        m_callback          = nullptr;
        m_delay             = defaultDelay;
        m_isWaitingCanceled = true;
    }
    m_cv.notify_one();
}

void PostExec::startMainLoop() {
    while (m_isMainLoopRunning) {
        std::unique_lock<std::mutex> lck(m_mutex);

        m_cv.wait_for(lck, m_delay,
                      [this] { return m_isWaitingCanceled == true; });

        if (m_isWaitingCanceled == false && m_callback != nullptr) {
            m_callback();
            m_callback = nullptr;
            m_delay    = defaultDelay;
        }

        m_isWaitingCanceled = false;
    }
}