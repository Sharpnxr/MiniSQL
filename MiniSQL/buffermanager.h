#pragma once
#include <iostream>
#include <math.h>
#include <Windows.h>
#include <atltime.h>
#include <atlstr.h>
#include <string>
#include <cstring>
#include "catalogmanager.h"
#include "indexmanager.h"
#include "recordmanager.h"
#include "Public.h"


using namespace std;

#define BLOCKSIZE 4096
#define NUMOFBLOCK 100

class RecordManager;
class CatalogManager;
class IndexManager;
class block;

//---------class buffermanager------------

class BufferManager
{
	block* BufBlock[NUMOFBLOCK];	//	总共有100个block，每个block的大小为4096byte

	int getFreeBlock();		// 用于搜寻一个空的block

	CatalogManager* cm;
public:
	BufferManager();
	BufferManager(CatalogManager* CM);
	~BufferManager();

	//给record manager 和 index manager 的接口，提供table文件（表）中的第num个block
	block* block_GET(string table, int num);

	//给record manager 的接口，在table中添加record，大小为size
	tuple_Entree *record_Insert(string table, char* record, int size);

	//给record manager 的接口，删除第blockNum个block中的第index条记录
	void record_Delete(int blockNum, int recordNum);

	//给record manager 的接口，执行drop table语句
	int table_Drop(string table);

};

//------------class buffermanager end------------

//------------class block------------------

class block
{
	char Block[BLOCKSIZE];	//每个block的大小是4096byte，即4KB
	string FileName;// 所在文件名
	int Num;		// 是指本block是整个文件中的第Num个block（而非buffer中的下标）
	bool Dirty;		// 是否为脏块
	bool Writen;	// 是否被写（否则是空块）
	CTime Time;		// 最后一次的访问时间（用于模拟LRU）
public:
	block();
	~block();

	bool record_Insert(char*, int size);

	bool record_Delete(int index);	//给record manager 的接口，用于执行delete 语句

	char* record_GET(int index);

	char* blockHeadPointer_GET();	//返回本block的头指针

	int getRecordPosition(int index);
	void setRecordPosition(int index, int Position);

	int getRecordSize(int index);
	void setRecordSize(int index, int size);

	void setFileName(string name);
	string getFileName();

	int getNum();
	void setNum(int num);

	bool isDirty();
	void setDirty();
	void cleanDirty();

	bool isWriten();
	void setWriten();
	void cleanWriten();

	void move(int NewPosition, int OldPosition, int size);

	void setRecordCount(int count);	//把record中的count变量存为block的前四个字节（即int型）
	int getRecordCount();			//count是记录block中存储的record的个数

	CTime getTime();		//得到访问时间
	void setTime(CTime t1);

	int getFreeSizeEnd();	//找到本block里空余空间的尾部，并返回
	void setFreeSpaceEnd(int place);

	int getSizeOfFreeSpace();	//获知本block内剩余的空余空间

	void restoreSpace(int index);	//回收被删除的record所占的空间

	bool readBlockFromDisk(string table, int num);	//读取磁盘中table文件（表）的第num个block
	bool writeBlockToDisk(string table, int num);	//写入~

	void initial();	//清空本block

};

//block的前四个字节（1~4），存储一个int类型，此int代表本block中record的个数
inline int block::getRecordCount()
{
	int* Begin = (int*)Block;
	return Begin[0];
}

//手动设置record总数量
inline void block::setRecordCount(int count)
{
	int * Begin = (int*)Block;
	Begin[0] = count;
}

//block的第5~8个字节存储一个int型下标，下标指向本block的空余空间的结尾
inline int block::getFreeSizeEnd()
{
	int* Begin = (int*)Block;
	return Begin[1];
}

//手动设置空余空间结尾
inline void block::setFreeSpaceEnd(int place)
{
	int* Begin = (int *)Block;
	Begin[1] = place;
}

//......
inline char* block::blockHeadPointer_GET()
{
	return Block;
}

//每个record在block的前端有记录，分别是大小size和record的头下标，且
//record的相关记录从Block[2]开始记起，所以寻找下标信息为Block[2n+1]
inline int block::getRecordPosition(int index)
{
	int* Begin = (int *)Block;
	return Begin[index * 2 + 1];
}

inline void block::setRecordPosition(int index, int Position)
{
	int *Begin = (int*)Block;
	Begin[index * 2 + 1] = Position;
}

//size存在头下标之前，故其信息位置为Block[2n]
inline int block::getRecordSize(int index)
{
	int* Begin = (int*)Block;
	return Begin[index * 2];
}

inline void block::setRecordSize(int index, int size)
{
	int*Begin = (int*)Block;
	Begin[index * 2] = size;
}

inline void block::setFileName(string name)
{
	FileName = name;
}

inline string block::getFileName()
{
	return FileName;
}

inline int block::getNum()
{
	return Num;
}

inline void block::setNum(int num)
{
	Num = num;
}

inline bool block::isDirty()
{
	return Dirty;
}

inline void block::setDirty()
{
	Dirty = true;
}

inline void block::cleanDirty()
{
	Dirty = false;
}

inline CTime block::getTime()
{
	return this->Time;
}

inline void block::setTime(CTime t1)
{
	this->Time = t1;
}

inline bool block::isWriten()
{
	return this->Writen;
}

inline void block::setWriten()
{
	this->Writen = true;
}

inline void block::cleanWriten()
{
	this->Writen = false;
}
