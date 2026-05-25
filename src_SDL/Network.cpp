#include "Network.h"
#include <iostream>
#include <zlib.h>
#include <ixwebsocket/IXNetSystem.h>

NetworkManager::NetworkManager(const std::string& url) : url(url) {
    ix::initNetSystem();
}

NetworkManager::~NetworkManager() {
    webSocket.stop();
    ix::uninitNetSystem();
}

void NetworkManager::onData(DataCallback callback) {
    callbacks.push_back(callback);
}

void NetworkManager::connect() {
    webSocket.setUrl(url);

    webSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Open) {
            std::cout << "Sunucuya başarıyla bağlanıldı." << std::endl;
            connected = true;
        }
        else if (msg->type == ix::WebSocketMessageType::Message) {
            if (msg->binary) {
                const std::string& str = msg->str;
                if (str.empty()) return;

                uint8_t isCompressed = str[0];
                if (isCompressed == 1) {
                    // Decompress (raw inflate)
                    z_stream strm = {};
                    strm.next_in = (Bytef*)str.data() + 1;
                    strm.avail_in = str.size() - 1;

                    // -15 for raw inflate window bits
                    if (inflateInit2(&strm, -15) != Z_OK) {
                        std::cerr << "Error init inflate" << std::endl;
                        return;
                    }

                    std::vector<uint8_t> outBuffer(4096);
                    std::vector<uint8_t> result;

                    int ret;
                    do {
                        strm.next_out = outBuffer.data();
                        strm.avail_out = outBuffer.size();
                        ret = inflate(&strm, Z_NO_FLUSH);
                        
                        if (ret != Z_OK && ret != Z_STREAM_END && ret != Z_BUF_ERROR) {
                            std::cerr << "Inflate error: " << ret << std::endl;
                            break;
                        }
                        
                        size_t have = outBuffer.size() - strm.avail_out;
                        result.insert(result.end(), outBuffer.begin(), outBuffer.begin() + have);
                        
                    } while (strm.avail_out == 0);

                    inflateEnd(&strm);

                    std::lock_guard<std::mutex> lock(queueMutex);
                    messageQueue.push(std::move(result));
                } else {
                    // Uncompressed
                    std::vector<uint8_t> result(str.begin() + 1, str.end());
                    
                    std::lock_guard<std::mutex> lock(queueMutex);
                    messageQueue.push(std::move(result));
                }
            } else {
                std::cout << "Metin veri alındı: " << msg->str << std::endl;
            }
        }
        else if (msg->type == ix::WebSocketMessageType::Error) {
            std::cerr << "WebSocket Hatası: " << msg->errorInfo.reason << std::endl;
        }
        else if (msg->type == ix::WebSocketMessageType::Close) {
            std::cout << "Bağlantı kapandı. Yeniden bağlanmayı deneyin." << std::endl;
            connected = false;
        }
    });

    webSocket.start();
}

void NetworkManager::send(const std::vector<uint8_t>& buffer) {
    if (!connected || buffer.empty()) return;

    // Compress using raw deflate
    z_stream strm = {};
    if (deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        std::cerr << "Error init deflate" << std::endl;
        return;
    }

    strm.next_in = (Bytef*)buffer.data();
    strm.avail_in = buffer.size();

    std::vector<uint8_t> outBuffer(buffer.size() + 256);
    std::vector<uint8_t> compressedData;

    int ret;
    do {
        strm.next_out = outBuffer.data();
        strm.avail_out = outBuffer.size();
        ret = deflate(&strm, Z_FINISH);
        
        size_t have = outBuffer.size() - strm.avail_out;
        compressedData.insert(compressedData.end(), outBuffer.begin(), outBuffer.begin() + have);
    } while (ret == Z_OK);

    deflateEnd(&strm);

    std::string payload;
    payload.reserve(1 + compressedData.size());
    payload.push_back(1); // Compressed flag = 1
    payload.append((char*)compressedData.data(), compressedData.size());

    webSocket.sendBinary(payload);
}

void NetworkManager::poll() {
    std::vector<std::vector<uint8_t>> currentMessages;
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        while (!messageQueue.empty()) {
            currentMessages.push_back(std::move(messageQueue.front()));
            messageQueue.pop();
        }
    }

    for (const auto& msg : currentMessages) {
        for (auto& cb : callbacks) {
            cb(msg);
        }
    }
}
