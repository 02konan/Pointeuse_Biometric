#ifndef __HTTP_CURL_H__
#define __HTTP_CURL_H__

#include <string>
#include "curl/curl.h"

class CHttpClient
{
public:
	CHttpClient(void);
	~CHttpClient(void);

public:
	/**
	* @brief HTTP POST请求
	* @param strUrl 输入参数,请求的Url地址,如:http://www.baidu.com
	* @param strPost 输入参数,使用如下格式para1=val1&para2=val2&…
	* @param strResponse 输出参数,返回的内容
	* @return 返回是否Post成功
	*/
	int Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse);

	/**
	* @brief HTTP GET请求
	* @param strUrl 输入参数,请求的Url地址,如:http://www.baidu.com
	* @param strResponse 输出参数,返回的内容
	* @return 返回是否Post成功
	*/
	int Get(const std::string & strUrl, std::string & strResponse);

	/**
	* @brief HTTPS POST请求,无证书版本
	* @param strUrl 输入参数,请求的Url地址,如:https://www.alipay.com
	* @param strPost 输入参数,使用如下格式para1=val1&para2=val2&…
	* @param strResponse 输出参数,返回的内容
	* @param pCaPath 输入参数,为CA证书的路径.如果输入为NULL,则不验证服务器端证书的有效性.
	* @return 返回是否Post成功
	*/
	int Posts(const std::string & strUrl, const std::string & strPost, std::string & strResponse, const char * pCaPath = NULL);

	/**
	* @brief HTTPS GET请求,无证书版本
	* @param strUrl 输入参数,请求的Url地址,如:https://www.alipay.com
	* @param strResponse 输出参数,返回的内容
	* @param pCaPath 输入参数,为CA证书的路径.如果输入为NULL,则不验证服务器端证书的有效性.
	* @return 返回是否Get成功
	*/
	int Gets(const std::string & strUrl, std::string & strResponse, const char * pCaPath = NULL);

    typedef CURL*(*CurlLib_curl_easy_init)(void);
    typedef CURLcode(*CurlLib_curl_easy_setopt)(CURL *curl, CURLoption option, ...);
    typedef CURLcode(*CurlLib_curl_easy_perform)(CURL *curl);
    typedef void(*CurlLib_curl_easy_cleanup)(CURL *curl);

    CurlLib_curl_easy_init          m_fnCurllInit;
    CurlLib_curl_easy_setopt        m_fnCurlSetOpt;
    CurlLib_curl_easy_perform       m_fnCurlPerform;
    CurlLib_curl_easy_cleanup       m_fnCurlCleanUp;

    

public:
	void SetDebug(bool bDebug);

private:
	bool m_bDebug;
};

#endif

