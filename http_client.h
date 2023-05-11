#pragma once

#include <vector>
#include <string>
#include <windows.h>
#include <wininet.h>



class http_client {
	/*HINTERNET h_internet_, h_connect_, h_request_, h_url_;*/
public:
	static std::string get_page(const std::string& url)
	{
		HINTERNET h_internet_, h_connect_;
		std::string result;
		char buffer[4096];
		memset(buffer, 0, 4096);
		std::vector<char> vb;

		h_internet_ = InternetOpen(TEXT("http_client"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if (h_internet_ != NULL) {
			h_connect_ = InternetOpenUrlA(h_internet_, url.c_str(), NULL, 0, 0, NULL);
			if (h_connect_ != NULL) {
				DWORD dwByteRead = 0;
				do 	{
					InternetReadFile(h_connect_, buffer, sizeof(buffer) - 1, &dwByteRead);
					vb.insert(vb.end(), buffer, buffer + dwByteRead);
				} while (dwByteRead);

				InternetCloseHandle(h_connect_);
			}
			InternetCloseHandle(h_internet_);
		}
		result.append(vb.begin(), vb.end());
		return result;
	}
	static std::vector<unsigned char> get_binary_page(std::string url)
	{
		HINTERNET h_internet_, h_connect_;
		char buffer[4096];
		memset(buffer, 0, 4096);
		std::vector<unsigned char> vb;

		h_internet_ = InternetOpen(TEXT("http_client"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if (h_internet_ != NULL) {
			h_connect_ = InternetOpenUrlA(h_internet_, url.c_str(), NULL, 0, 0, NULL);
			if (h_connect_ != NULL) {
				DWORD dwByteRead = 0;
				do {
					InternetReadFile(h_connect_, buffer, sizeof(buffer) - 1, &dwByteRead);
					vb.insert(vb.end(), buffer, buffer + dwByteRead);
				} while (dwByteRead);
				InternetCloseHandle(h_connect_);
			}
			InternetCloseHandle(h_internet_);
		}
		return vb;
	}
};