#ifndef __HTTP_CLIENT__
#define __HTTP_CLIENT__

#include <WinInet.h>

class http_client
{
	HINTERNET hInternet, hConnect, hRequest, hUrl;
public:
	std::string get_page(std::string url)
	{
		std::string result;
		char buffer[4096];
		memset(buffer, 0, 4096);
		std::vector<char> vb;

		hInternet = InternetOpen(TEXT("3d_engine"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if (hInternet != NULL)
		{
			hConnect = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, 0, NULL);
			if (hConnect != NULL)
			{
				DWORD dwByteRead = 0;
				do
				{
					InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &dwByteRead);
					vb.insert(vb.end(), buffer, buffer + dwByteRead);
				} while (dwByteRead);

				InternetCloseHandle(hConnect);
			}
			InternetCloseHandle(hInternet);
		}
		result.append(vb.begin(), vb.end());
		return result;
	}
	std::vector<unsigned char> get_binary_page(std::string url)
	{
		char buffer[4096];
		memset(buffer, 0, 4096);
		std::vector<unsigned char> vb;

		hInternet = InternetOpen(TEXT("3d_engine"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if (hInternet != NULL)
		{
			hConnect = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, 0, NULL);
			if (hConnect != NULL)
			{
				DWORD dwByteRead = 0;
				do
				{
					InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &dwByteRead);
					vb.insert(vb.end(), buffer, buffer + dwByteRead);
				} while (dwByteRead);

				InternetCloseHandle(hConnect);
			}
			InternetCloseHandle(hInternet);
		}
		return vb;
	}
};

#endif //__HTTP_CLIENT__