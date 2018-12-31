#include <iostream>
#include <math.h>
#include <Windows.h>
#include <atltime.h>
#include <atlstr.h>
#include <string>
#include "recordmanager.h"
#include "buffermanager.h"
using namespace std;



int getNumOfBlock(string table)//直接询问一个table用了多少个block
{
	HANDLE hFile;
	CString FileName = CString(table_File_GET(table).c_str());
	hFile = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		cout << "Error In Opening File " << table << endl;
	}
	double temp = GetFileSize(hFile, 0) / BLOCKSIZE;
	long long int FileSize = (long long)ceil(temp);	//除一下，然后向上取整，得出用的block数量
	return FileSize;
}

block::block()
{
	this->initial();
}

block::~block()
{
	string table = this->getFileName();
	int num = this->getNum();
	this->writeBlockToDisk(table, num);	//当buffer中的一个block被释放（因为被替换）时，将文件中的第Num个（也就是这个）block更新
}

void block::initial()
{
	for (int i = 8; i < BLOCKSIZE; i++)
		Block[i] = '\0';
	FileName = " ";
	Num = 0;
	Dirty = false;
	Writen = false;
	Time = 0;
	int *Begin = (int *)Block;
	Begin[0] = 0;
	Begin[1] = BLOCKSIZE - 1;//初始化可用空间结尾下标
}

void block::restoreSpace(int index)
{
	char* content = this->blockHeadPointer_GET();
	int* Begin = (int*)content;
	int Count = this->getRecordCount();
	int Final = Count;	//在更新freespace信息前先取当前的record数量
	int MoveSpaceForRecord = 0;
	int RecordDeleted = 0;
	int size = 0;
	for (int i = 1; i <= Count; i++)
	{
		int Tmpsize = this->getRecordSize(i);
		if (Tmpsize > 0) {
			size = Tmpsize;
			break;
		}
	}
	if (size == 0) return;
	for (int i = index; i <= Count; i++)
	{
		int TempSize = this->getRecordSize(i);
		if (!TempSize)	//如果找到的record实际已经被删掉了（即tempsize=0）
		{
			if (i != index) continue;
			Final--;
			RecordDeleted++;
			if (i < Count)//如果不是最后一个record被删掉了（最后一个就不用算后移量了）
			{
				//int lastPosition;
				//if (i == 1) lastPosition = 4096;
				//	else lastPosition = this->getRecordPosition(i);
				//MoveSpaceForRecord += this->getRecordPosition(i+1) - lastPosition);
				MoveSpaceForRecord += size;
			}
		}
		else
			//如果是有效的块，则向后移动，填补其后面已经被删掉的位置
		{
			int OldPosition = this->getRecordPosition(i);
			int NewPosition = OldPosition + MoveSpaceForRecord;
			this->move(NewPosition, OldPosition, TempSize);		//把删掉的record前面（也就是序号在其后面）的record都向后移动填补空位
			//this->setRecordSize(i, TempSize);	//recorddeleted是在此record后面已经被删掉了的数量
			this->setRecordPosition(i, NewPosition);
		}
	}
	//this->setRecordCount(Final);	//最终，更新实际剩余的record数量
	this->setFreeSpaceEnd(this->getRecordPosition(Count) - 1);	//再更新指向空余位结尾的下标
}

//前移的实际操作
void block::move(int NewPosition, int OldPosition, int size)
{
	char* Begin = this->blockHeadPointer_GET();
	for (int i = size - 1; i >= 0; i--)
		Begin[NewPosition + i] = Begin[OldPosition + i];
}


int block::getSizeOfFreeSpace()
{
	int* Begin = (int*) this->blockHeadPointer_GET();
	int EndOfFreeSpace = Begin[1];
	int RecordCount = Begin[0];
	int size = EndOfFreeSpace - 8 * RecordCount - 2 * 4;	//一个record在block前端对应一个size(int)和一个position(int),即8位，最前面还有一个recordcount和一个freespaceend
	return size;
}

bool block::readBlockFromDisk(string table, int blocknum)
{
	HANDLE hFile;
	CString FileName = CString(table_File_GET(table).c_str());
	hFile = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		cout << "Couldnot open file error" << endl;
		return false;
	}

	long size = (GetFileSize(hFile, 0) / BLOCKSIZE);

	int FileOffset = blocknum*BLOCKSIZE;

	if (blocknum > size)
	{
		cout << "Error, Wrong Block To Load" << endl;
		return NULL;
	}
	else
	{
		unsigned long Actual;
		SetFilePointer(hFile, FileOffset, 0, FILE_BEGIN);
		ReadFile(hFile, this->Block, BLOCKSIZE, &Actual, NULL);
		this->cleanDirty();	//刚从disk读入buffer中的block不是脏的
		this->setWriten();	//本block被写过了
		this->setFileName(table);
		this->setNum(blocknum);	//Num作为区分buffer中block的唯一标准
		CloseHandle(hFile);
		return true;
	}
}

bool block::writeBlockToDisk(string table, int num)
{
	if (this->isDirty())
	{
		HANDLE hFile;
		CString FileName = CString(table_File_GET(table).c_str());
		hFile = CreateFile(FileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			cout << "Error in opening file" << this->getFileName() << endl;
			return false;
		}
		unsigned long Actual;
		SetFilePointer(hFile, (num)*BLOCKSIZE, 0, FILE_BEGIN);
		WriteFile(hFile, this->Block, BLOCKSIZE, &Actual, NULL);
		this->cleanDirty();
		CloseHandle(hFile);
		return true;
	}
}

bool block::record_Insert(char* record, int size)
{
	int temp = this->getSizeOfFreeSpace();	//看当前block里面还有多少空余位
	if (temp > size + 8)	//大于则可以放置
	{
		int OldEnd = this->getFreeSizeEnd();
		int Count = this->getRecordCount();
		Count++;
		this->setRecordPosition(Count, OldEnd - size + 1);	//第count个record的头指针为原本的最后空余位-size+1
		int Position = this->getRecordPosition(Count);
		for (int i = 0; i < size; i++)
			this->Block[Position + i] = record[i];	//总共size位，按位赋值，即拷贝新的record进入block
		this->setRecordCount(Count);		//新的count(count+1)存入block头
		this->setRecordSize(Count, size);	//把新record的size也存入
		this->setFreeSpaceEnd(Position - 1);
		this->setDirty();
		this->setWriten();
		return true;
	}
	else
		return false;
}

bool block::record_Delete(int index)
{
	this->setRecordSize(index, 0);	//把block前部的本record索引size归零，意思是它没了，就相当于删掉了，然后用restoreSpace回收空间并后移
	this->setDirty();
	this->restoreSpace(index);
	return true;
}

char* block::record_GET(int index)	//得到block中的第index个record，其中1<=index<=this->getRecordCount();
{
	int* Begin = (int*) this->blockHeadPointer_GET();
	int startPosition = Begin[index * 2 + 1];
	return this->blockHeadPointer_GET() + startPosition;
}

BufferManager::BufferManager()
{
	for (int i = 0; i < NUMOFBLOCK; i++)
	{
		BufBlock[i] = new block();
	}
}

BufferManager::BufferManager(CatalogManager* CM) 
{
	cm = CM;
	for (int i = 0; i < NUMOFBLOCK; i++)
	{
		BufBlock[i] = new block();
	}
}

BufferManager::~BufferManager()
{
	for (int i = 0; i < NUMOFBLOCK; i++)
		delete BufBlock[i];
}

int BufferManager::getFreeBlock()	//从buffer中找到一个可用的block位，
{
	int ToReplace = 0;
	CTime Tm = BufBlock[0]->getTime();	//记录buffer中第0个block（最上面的）最后一次被访问的时间
	for (int i = 0; i < NUMOFBLOCK; i++)
	{
		if (BufBlock[i]->isWriten() == false)	//如果这个block是没有被写入（也就是空）的
			return i;
		if (BufBlock[i]->getTime() < Tm)		//如果是有值的block，检查其最后访问时间，如果访问时间更靠前，则这个被优先替换
			ToReplace = i;
	}
	//遍历一遍后，如果有空的直接用空的，没有空的则替换掉最早被访问的块（就是很久没被访问的那个），即LRU
	if (BufBlock[ToReplace]->isDirty())
		BufBlock[ToReplace]->writeBlockToDisk(BufBlock[ToReplace]->getFileName(),
			BufBlock[ToReplace]->getNum());	//如果要被替换掉的block是脏的，则把这个块写回去
	BufBlock[ToReplace]->initial();
	return ToReplace;
}

tuple_Entree *BufferManager::record_Insert(string table, char *record, int size)
{
	tuple_Entree *temp = new tuple_Entree();
	for (int i = 0; i < NUMOFBLOCK; i++)
	{
		if (BufBlock[i]->getFileName() == table)
		{
			if (BufBlock[i]->record_Insert(record, size))	//如果添加成功了，就可以退出了
			{
				temp->block_id = BufBlock[i]->getNum();
				temp->id = BufBlock[i]->getRecordCount();
				temp->entry_Tuple = record;
				return temp;
			}
		}
	}
	//下面是没有添加成功的情况
	int Position = getFreeBlock();	//向buffer申请一个block来写入，没有空的就用LRU腾出来一个
	HANDLE hFile;
	CString FileName = CString(table_File_GET(table).c_str());
	hFile = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		cout << "ERROR in opening file " << table << endl;
		temp->block_id = -1;
		temp->id = -1;
		temp->entry_Tuple = NULL;
		return temp;
	}

	double tempsize = GetFileSize(hFile, 0) / BLOCKSIZE;
	long long FileSize = (long long)ceil(tempsize);
	cm->AddTableBlockNum(table);

	BufBlock[Position]->initial();
	BufBlock[Position]->record_Insert(record, size);	//调用这个block的record_Insert
	BufBlock[Position]->setFileName(table);
	BufBlock[Position]->setNum(FileSize);
	BufBlock[Position]->setDirty();
	BufBlock[Position]->setWriten();
	BufBlock[Position]->writeBlockToDisk(table, FileSize);
	CloseHandle(hFile);
	temp->block_id = BufBlock[Position]->getNum();
	temp->id = BufBlock[Position]->getRecordCount();
	temp->entry_Tuple = record;
	return temp;
}

int BufferManager::table_Drop(string table)
{
	for (int i = 0; i < NUMOFBLOCK; i++)
	{
		if (BufBlock[i]->getFileName() == table)
		{
			BufBlock[i]->initial();
		}
	}

	if (remove(table.c_str()) == -1)
		return -1;
	else
		return 0;
}

void BufferManager::record_Delete(int blockNum, int index)
{
	BufBlock[blockNum]->setRecordSize(index, 0);
	BufBlock[blockNum]->setDirty();
	BufBlock[blockNum]->restoreSpace(index);
}

block* BufferManager::block_GET(string table, int num)	//这个num是指文件中的第num个block，本函数检测这个block是否在buffer中，不在则从disk读取
{
	if (num < 0)
	{
		cout << "Error, Wrong Block To Load" << endl;
		return NULL;
	}
	for (int i = 0; i < NUMOFBLOCK; i++)
	{
		if ((BufBlock[i]->getFileName() == table) && (BufBlock[i]->getNum() == num))
			return BufBlock[i];
	}
	//没找到，则从文件中读取
	int Position = getFreeBlock();
	BufBlock[Position]->readBlockFromDisk(table, num);
	return BufBlock[Position];
}

