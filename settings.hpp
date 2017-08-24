#ifndef SETTINGS_H
#define SETTINGS_H

#include "inc/json.hpp"
#include <openssl/sha.h>
#include <bitset>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <curl/curl.h>

#define PRINTCURLINFO 0// switch on for curl to use verbose output

using namespace nlohmann;
using namespace std;

typedef vector<pair<string, string>> paramVect;

namespace utilities {

	enum RequestMethod {PUBLIC, PRIVATE};

	static string sha256(string s) {
		SHA256_CTX ctx;
		SHA256_Init(&ctx);
		SHA256_Update(&ctx, s.c_str(), s.size());

		unsigned char result[33];
		SHA256_Final(result, &ctx);
		result[32] = '\0';
		return string((char*)result);
	}

	static unsigned char* sha512hmac(string& key, string& data, unsigned int& len) {
		unsigned char* k = (unsigned char*)key.c_str();
		unsigned char* d = (unsigned char*)data.c_str();
		unsigned char* dgst = HMAC(EVP_sha512(), k, key.size(), d, data.size(), 0, &len);

		return dgst;
	}

	static string getQueryString(paramVect& vars, bool isGet = true) {
		string q = "";
		if (isGet) q.append("?");
		for (vector<pair<string, string>>::iterator it = vars.begin(); it != vars.end(); it++) {
			if (it != vars.begin()) q.append("&");
			q.append(it->first);
			q.append("=");
			q.append(it->second);
		}
		return q;
	}

	static string makeHttpHeaderString(RequestMethod method, string path, paramVect& vars) {
		string header;
		if (method == RequestMethod::PUBLIC)
			header = "GET /";
		else
			header = "POST /";
		header.append(path);
		header.append(" HTTP/1.1");
		for (vector<pair<string, string>>::iterator it = vars.begin(); it != vars.end(); it++) {
			header.append("\r\n");
			header.append(it->first);
			header.append(": ");
			header.append(it->second);
		}
		header.append("\r\n\r\n");
		return header;
	}
	//callback function for curl response
	static string res;
	static size_t writer(char* buf, size_t size, size_t nmemb, void* up) {
		for (unsigned i = 0; i < size*nmemb; i++)
			res.push_back(buf[i]);
		return size*nmemb;
	}

	static string Base64Decode(const char* msg) { //Decodes a base64 encoded string
		BIO *bio, *b64;

		size_t decodeLen = (strlen(msg) * 3) / 4 - 2; // -2 if api key has == at the end
		unsigned char* buffer = (unsigned char*)malloc(decodeLen);
		buffer[decodeLen] = '\0';

		bio = BIO_new_mem_buf(msg, -1);
		b64 = BIO_new(BIO_f_base64());
		bio = BIO_push(b64, bio);

		BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Do not use newlines to flush buffer
		BIO_read(bio, buffer, strlen(msg));
		BIO_free_all(bio);

		return string((char*)buffer);
	}

	static string Base64Encode(const unsigned char* buffer, size_t length) { //Encodes a binary safe base 64 string
		BIO *bio, *b64;
		BUF_MEM *bufferPtr;

		b64 = BIO_new(BIO_f_base64());
		bio = BIO_new(BIO_s_mem());
		bio = BIO_push(b64, bio);

		BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Ignore newlines - write everything in one line
		BIO_write(bio, buffer, length);
		BIO_flush(bio);
		BIO_get_mem_ptr(bio, &bufferPtr);
		BIO_set_close(bio, BIO_NOCLOSE);
		BIO_free_all(bio);
		int encodedLen = ((length + 2) / 3) * 4;
		string encoded = (*bufferPtr).data;
		if (encoded.size() > encodedLen)
			encoded = encoded.substr(0, encodedLen);
		return encoded;
	}
}
#endif
