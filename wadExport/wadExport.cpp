#include "wadExport.h"

vector<Wad_Struct> v;
map <CString, CString> mp;

UINT32 wad_count;//WAD文件数量
CString wad_filePath;//WAD文件路径

int main()
{
	//*********************************************************** Init
	HMODULE hdll;
	hdll = LoadLibraryA("libzstd.dll");
	if (hdll == 0)
	{
		FreeLibrary(hdll);
		cout << "load error not libzstd.dll ; 模块加载失败 libzstd.dll\n" << endl;
		return 0;
	}
	ZSTD_getDecSiz = (ZSTD_getDecompressedSize)GetProcAddress(hdll,"ZSTD_getDecompressedSize");
	if (ZSTD_getDecSiz == 0)
	{
		FreeLibrary(hdll);
		cout << "load error not ZSTD_decompress ; 找不到函数 ZSTD_getDecompressedSize\n" << endl;
		return 0;
	}
	ZSTD_dec = (ZSTD_decompress)GetProcAddress(hdll,"ZSTD_decompress");//ZSTD_decompress
	if (ZSTD_dec == 0)
	{
		FreeLibrary(hdll);
		cout << "load error not ZSTD_decompress ; 找不到函数 ZSTD_decompress\n" << endl;
		return 0;
	}

	//*********************************************************** Start
	cout << "Are you ready\n" << endl;

	load_hashes("hashes.txt");

	int err = openfile("Map22LEVELS.wad.client");//打开文件

	if (err == 0)
	{
		CreateDirectory("new", NULL);//创建目录
		int err2 = export_all("new");//导出文件
	}else
	{
		cout << err << endl;
	}
	return 0;
}

int openfile(const char * in_filePath)//打开WAD文件
{
	ifstream in(in_filePath, ios::binary);
	if (!in.is_open())
	{
		cout << "Open error The file is useing ; 打开失败，WAD文件被占用" << endl;
		return 1;
	}
		
	int v_DataOffset = 0;//数据偏移	
	UINT32 ver_header = 0;//版本头
	UINT32 ver30 = 0x00035752; 	

	in.read((char*)&ver_header, 4);
	if (ver_header == ver30)//判断文件版本
	{
		v_DataOffset = 268;
	}else
	{
		in.close();
		cout << "Read error no is version 3.0 ; 读取失败，文件不是3.0版本" << endl;
		return 2;
	}

	wad_filePath = in_filePath;
	in.seekg(0, ios::beg);
	in.seekg(0, ios::end);
	long long fileSize = in.tellg();//取文件大小
	//cout << "fileSize:" << fileSize << endl;
	
	if (fileSize > 1024*1024*200)
	{
		in.close();
		cout << "fileSize error of bounds 200mb ; 错误 文件大于200mb 超出预算" << endl;
		return 3;
	}

	in.seekg(v_DataOffset, ios::beg);
	in.read((char*)&wad_count, 4);//取文件数量
	//cout << "wad_count:" << wad_count << endl;

	for (UINT32 i = 0; i < wad_count; i++)
	{
		if (i == 0)
		{
			in.seekg(v_DataOffset + 4 , ios::beg);
		}
		else
		{
			UINT32 Offset = (v_DataOffset + 4) + 32*i;//设置偏移值
			in.seekg(Offset, ios::beg);
		}

		UINT64 xxhash = 0;				
		in.read((char*)&xxhash, 8);//条目名	
		char xxhash_tem[64];
		sprintf_s(xxhash_tem,64, "%016llx", xxhash);
		CString xxhash_str = xxhash_tem;		

		UINT32 DataOffset = 0;
		in.read((char*)&DataOffset, 4);//数据偏移

		UINT32 Compressed = 0;
		in.read((char*)&Compressed, 4);//压缩大小

		UINT32 Uncompressed = 0;
		in.read((char*)&Uncompressed, 4);//未压缩大小

		UINT8 Ctype = 0;
		in.read((char*)&Ctype, 1);//压缩类型

		//**********************************************************************
		Wad_Struct *p = new Wad_Struct();
		p->xxHash_str = xxhash_str;//16进制文件名

		CString mp_name;
		if(mp.count(xxhash_str) != 0)
		{
			mp_name = mp[xxhash_str];//如果map容器键存在就使用哈希表的文件名
		}else
		{
			mp_name = xxhash_str;//如果不存在就是要文本哈希名
		}	

		if (mp_name.GetLength() <= MAX_PATH/2)//如果文件名超过130个字节，文件名取16位文本哈希
		{
			p->FileName = mp_name;	
		}else
		{
			p->FileName = xxhash_str;
		}
		
		p->DataOffset = DataOffset;
		p->CompressedSize = Compressed;
		p->UncompressedSize = Uncompressed;
		p->Type = Ctype;
		//cout << "条目名:"<< p->FileName << endl;
		v.push_back(*p);
		delete p;		
	}
	in.close();
	return 0;
}

//导出全部文件
int export_all(const char * out_directory)
{	
	for (vector <Wad_Struct> ::iterator it = v.begin(); it != v.end(); it++)//历遍容器
	{
		CString tem = out_directory;
		CString FileName = tem + "\\" + it->FileName;//目录加上条目名
		UINT8 Type = it->Type;
		UINT32 DataOffset = it->DataOffset;
		UINT32 c = it->CompressedSize;
		UINT32 u = it->UncompressedSize;

		UINT32 bufSize;
		char *c_buf = 0;
		if (Type == 3) 
		{
			c_buf = new char[c]();
			bufSize = c;					
		}else if (Type == 1)
		{
			c_buf = new char[c]();
			bufSize = c;
		}else if (Type == 0)
		{
			c_buf = new char[u]();
			bufSize = u;
		}else
		{
			continue;
		}	

		ifstream in(wad_filePath, ios::binary);
		in.seekg(DataOffset, ios::beg);//读取数据
		in.read(c_buf, bufSize);
		in.close();

		CString FileName_tem = FileName;
		FileName_tem.Replace("/", "\\" );//转换斜杠
		//cout <<FileName_tem + "8" << endl;

		char *Path = FileName_tem.GetBuffer(0);//转为char*
		
		CString p1 = Path ;		
		FileName_tem.ReleaseBuffer();

		char* Name =PathFindFileNameA(Path);//取出文件名
		PathRemoveFileSpecA(Path);//删除文件名
		PathAddBackslashA(Path);//加上右斜杠
			
		MakeSureDirectoryPathExists(Path);//创建多级目录

		size_t retSize = ZSTD_getDecSiz(c_buf, bufSize);
		char* dec_buf = new char[retSize]();
		size_t decSize = ZSTD_dec(dec_buf, retSize,c_buf, bufSize);//解压数据

		ofstream out(p1, ios::binary);
		out.write(dec_buf, decSize);//写出文件
		out.close();
		
		delete dec_buf;
		delete c_buf;
	}
	return 0;
}

//加载哈希表
bool load_hashes(const char * filePath)
{	       	
	ifstream in(filePath, ios::binary);
	if (!in.is_open())
	{
		cout << "hashes load error 读取失败" << endl;
		return false;
	}	
	//取文件大小
	in.seekg(0, ios::end);
	long long fileSize = in.tellg();
	in.seekg(0,ios::beg);

	//动态申请内存，并将空间内容初始化为零
	char* c_buf = new char[(unsigned int)fileSize]();

	//这里有两个选择，前者以文本的方式一行一行的读进去，后者先读入整个文件，然后在内存里面处理，我选择后者（这样比较快）
	in.read(c_buf, fileSize);
	
	char c_lineBuf[204800];	//用一个变量保存一行的内容，200kb 20万多个字节，一般不会存在超过这个长度的行
	CString s_line, s_Left, s_Right;
	in.seekg(0,ios::beg);
	while (!in.eof())
	{
		in.getline(c_lineBuf,sizeof(c_lineBuf));	
		s_line = c_lineBuf;
		//cout << s_line << endl;
		s_Left = s_line.Left(16);//哈希表的左边部分
		s_Left.MakeLower();//转小写
		//cout << s_Left << endl;
		s_Right = s_line.Right(s_line.GetLength() - 17);//哈希表的右边部分					
		s_Right.Replace("\n", "");//去掉 Windos linux 各种换行符
		s_Right.Replace("\r", "");
		s_Right.Replace("\r\n", "");
		//cout << s_Right <<"8"<< endl;
		mp.insert(pair<CString, CString>(s_Left, s_Right));//把哈希表左右两段丢进map容器里面
	}	
	in.close();//关闭文件
	delete c_buf;//释放内存
	return true;
}