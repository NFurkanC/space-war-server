#pragma once
#include <vector>
#include <functional>
#include <string>
#include <memory>
#include <mutex>
#include <queue>
#include <ixwebsocket/IXWebSocket.h>

class NetworkManager {
public:
    using DataCallback = std::function<void(const std::vector<uint8_t>&)>;

    NetworkManager(const std::string& url);
    ~NetworkManager();
    
    void onData(DataCallback callback);
    void connect();
    void send(const std::vector<uint8_t>& buffer);

    void poll();

private:
    std::string url;
    std::vector<DataCallback> callbacks;
    ix::WebSocket webSocket;
    bool connected = false;

    std::mutex queueMutex;
    std::queue<std::vector<uint8_t>> messageQueue;
};
