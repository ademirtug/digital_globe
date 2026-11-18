#include "http_client.h"
#include <curl/curl.h>
#include <mutex>
#include <stdexcept>

std::shared_ptr<std::vector<unsigned char>> http_client::get_binary_page(const std::string& url) {
	static std::once_flag initialize;
	std::call_once(initialize, [] { if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) throw std::runtime_error("Could not initialize HTTPS."); });
	auto data = std::make_shared<std::vector<unsigned char>>();
	std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> request(curl_easy_init(), curl_easy_cleanup);
	if (!request) throw std::runtime_error("Could not create an HTTPS request.");
	curl_easy_setopt(request.get(), CURLOPT_URL, url.c_str());
	curl_easy_setopt(request.get(), CURLOPT_USERAGENT, "digital_globe/0.2 (https://github.com/ademirtug/digital_globe)");
	curl_easy_setopt(request.get(), CURLOPT_CONNECTTIMEOUT, 5L);
	curl_easy_setopt(request.get(), CURLOPT_TIMEOUT, 15L);
	curl_easy_setopt(request.get(), CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(request.get(), CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(request.get(), CURLOPT_MAXREDIRS, 3L);
	curl_easy_setopt(request.get(), CURLOPT_PROTOCOLS_STR, "https");
	curl_easy_setopt(request.get(), CURLOPT_REDIR_PROTOCOLS_STR, "https");
	curl_easy_setopt(request.get(), CURLOPT_FAILONERROR, 1L);
	curl_easy_setopt(request.get(), CURLOPT_WRITEDATA, data.get());
	curl_easy_setopt(request.get(), CURLOPT_WRITEFUNCTION, +[](char* bytes, size_t size, size_t count, void* context) -> size_t {
		auto& output = *static_cast<std::vector<unsigned char>*>(context);
		auto length = size * count;
		if (length > 8 * 1024 * 1024 || output.size() > 8 * 1024 * 1024 - length) return 0;
		try { output.insert(output.end(), bytes, bytes + length); } catch (...) { return 0; }
		return length;
	});
	if (curl_easy_perform(request.get()) != CURLE_OK) data->clear();
	return data;
}
