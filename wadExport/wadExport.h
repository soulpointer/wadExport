#pragma once
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <imagehlp.h>  
#pragma comment(lib,"imagehlp.lib") 

#include <atlstr.h> 
//MFC CString File header 
//为什么使用MFC风格的字串符类，我的前端是绑定在MFC为了更加兼容，可能对GCC平台不友好，适用VC
// VC++11 test 代码未经优化 在实际使用种可能会出现效率很慢 属于正常 需要自己优化

using namespace std;

//zstd函数声明-动态加载
typedef size_t (*ZSTD_getDecompressedSize)(const void* src, size_t srcSize);
typedef size_t (*ZSTD_decompress)( void* dst, size_t dstCapacity,const void* src, size_t compressedSize);
ZSTD_getDecompressedSize ZSTD_getDecSiz;
ZSTD_decompress ZSTD_dec;

//加载哈希表
bool load_hashes(const char * in_filePath);

//打开WAD文件
int openfile(const char * in_filePath);

//导出全部文件
int export_all(const char * out_directory);

//WAD数据结构
struct Wad_Struct
{
    CString FileName;//条目文件名
	CString xxHash_str;//条目文本哈希
	unsigned int DataOffset;//数据偏移
	unsigned int CompressedSize;//压缩后大小
	unsigned int UncompressedSize;//压缩前大小
	unsigned char Type;//条目类型   
};
