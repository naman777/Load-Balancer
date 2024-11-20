#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <curl/curl.h>

// Configuration
const std::string loadBalancerUrl = "http://127.0.0.1:8080"; // Load balancer address
const int numRequests = 100; // Number of requests to send

// Callback function to handle data received from HTTP response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userData) {
    size_t totalSize = size * nmemb;
    userData->append((char*)contents, totalSize);
    return totalSize;
}

// Function to make an HTTP request
void makeRequest(int requestNumber) {
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, loadBalancerUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Request #" << requestNumber << " failed: " 
                      << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << "Response #" << requestNumber << ": " << response << std::endl;
        }

        curl_easy_cleanup(curl);
    } else {
        std::cerr << "Failed to initialize CURL for request #" << requestNumber << std::endl;
    }
}

// Main function to send multiple requests
void sendRequests() {
    std::cout << "Sending " << numRequests << " requests to " << loadBalancerUrl << "..." << std::endl;
    std::vector<std::thread> threads;

    for (int i = 1; i <= numRequests; ++i) {
        threads.emplace_back(makeRequest, i);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "All requests completed." << std::endl;
}

// Entry point
int main() {
    curl_global_init(CURL_GLOBAL_ALL); // Initialize CURL globally
    sendRequests();
    curl_global_cleanup(); // Cleanup CURL globally
    return 0;
}
