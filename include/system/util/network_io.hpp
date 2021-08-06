#pragma once
#include <vector>
#include <map>
#include <string>
#include <utility>
#include <3ds.h>

// one instance per one url (once constructed, the url is not changeable)
struct NetworkStream {
	static constexpr u64 BLOCK_SIZE = 0x40000; // 256 KiB
	static constexpr u64 MAX_CACHE_BLOCKS = 100;
	
	u64 block_num = 0;
	std::string url;
	Handle downloaded_data_lock; // std::map needs locking when searching and inserting at the same time
	std::map<u64, std::vector<u8> > downloaded_data;
	bool whole_download = false;
	
	// anything above here is not supposed to be used from outside network_io.cpp and network_io.hpp
	u64 len = 0;
	volatile bool ready = false;
	volatile bool suspend_request = false;
	volatile bool quit_request = false;
	volatile bool error = false;
	volatile u64 read_head = 0;
	const char * volatile network_waiting_status = NULL;
	bool disable_interrupt = false;
	// used for livestreams
	int seq_head = -1;
	int seq_id = -1;
	bool livestream_eof = false;
	bool livestream_private = false;
	
	// if `whole_download` is true, it will not use Range request but download the whole content at once (used for livestreams)
	NetworkStream (std::string url, bool whole_download = false);
	
	double get_download_percentage();
	std::vector<double> get_buffering_progress_bar(int res_len);
	
	// check if the data of the current stream of range [start, start + size) is already downloaded and available
	bool is_data_available(u64 start, u64 size);
	
	// this function must only be called when is_data_available(start, size) returns true
	// returns the data of the stream of range [start, start + size)
	std::vector<u8> get_data(u64 start, u64 size);
	
	// this function is supposed to be called from NetworkStreamDownloader::*
	void set_data(u64 block, const std::vector<u8> &data);
};


// each instance of this class is paired with one downloader thread
// it owns NetworkStream instances, and the one with the least margin (as in proportion to the length of the entire stream) is the target of next downloading
class NetworkStreamDownloader {
private :
	static constexpr u64 BLOCK_SIZE = NetworkStream::BLOCK_SIZE;
	static constexpr u64 MAX_FORWARD_READ_BLOCKS = 50;
	static constexpr const char * USER_AGENT = "Mozilla/5.0 (Linux; Android 11; Pixel 3a) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.101 Mobile Safari/537.36";
	
	Handle streams_lock;
	std::vector<NetworkStream *> streams;
	
	bool thread_exit_reqeusted = false;
public :
	NetworkStreamDownloader ();
	
	// the pointer must be one that has been new-ed : it will be deleted once quit_request is made
	void add_stream(NetworkStream *stream);
	
	void request_thread_exit() { thread_exit_reqeusted = true; }
	void delete_all();
	
	void downloader_thread();
};
// 'arg' should be a pointer to an instance of NetworkStreamDownloader
void network_downloader_thread(void *arg);

// it's just useful
std::pair<std::string, httpcContext> access_http_get(std::string url, std::map<std::string, std::string> request_headers);
std::pair<std::string, httpcContext> access_http_get_modify_on_redirect(std::string &url, std::map<std::string, std::string> request_headers);
std::pair<std::string, httpcContext> access_http_post(std::string url, std::vector<u8> content, std::map<std::string, std::string> request_headers);

#define HTTP_STATUS_CODE_OK 200
#define HTTP_STATUS_CODE_NO_CONTENT 204
#define HTTP_STATUS_CODE_PARTIAL_CONTENT 206
