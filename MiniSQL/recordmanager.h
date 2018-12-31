#pragma once

#ifndef RECORDMANAGER_H_
#define RECORDMANAGER_H_
#include <string>
#include <vector>
#include "buffermanager.h"
#include "catalogmanager.h"
#include "indexmanager.h"
#include "Public.h"



using namespace std;

class BufferManager;
class CatalogManager;
class IndexManager;
class Condition;
class Attribute;
enum type;

class RecordManager
{
public:
	//Initializers
	RecordManager() {}
	RecordManager(BufferManager *BM, CatalogManager *CM, IndexManager *IM)
	{
		bm = BM;
		cm = CM;
		im = IM;
	}
	~RecordManager() {}
	BufferManager *bm;
	CatalogManager *cm;
	IndexManager *im;
	//Table Operations
	int table_Create(string name); //添加名为name的表格
	int table_Drop(string name); //删除名为name的表格
	//Index Operations
	int index_Create(string name, string attrib, string table_name); //添加表格名为name, 针对attrib参数的索引
	int index_Drop(string name, string attrib, string table_name);	//删除表格名为name，针对attrib参数的索引
	int record_Insert(string table, vector<string>* rec, vector<type>* typ);
	//Record Operations
	//添加一个记录，表格名为table，将每个记录按照创建表格时候的顺序进行插入，切成段放入容器，输出值为成功与否
	int record_Show(string table, vector<string>* attrib_list, Condition & conditions); //查询table内满足conditions条件，并显示attrib_list指定的内容，返回值为查询到的个数，查询结果直接显示；如果使用*查询直接在vector的第一项放“*”
	int record_Delete(string table, Condition & conditions); //删除table内满足conditions条件

	vector<Attribute> attrib_GET(string table);
	vector<Attribute> attrib_Convert(string table, vector<string>* attrib_list);
	type attrib_this_GET(string table, string attrib);
	int table_Size_GET(string TName);
private:
	vector<tuple_Entree>* record_Find(string table, Condition & conditions);
	vector<tuple_Entree>* record_Find(string table, Condition & conditions, vector<tuple_Entree>* tuples);
	vector<tuple_Entree>* record_Find_w_Index(string table, Condition & conditions, vector<tuple_Entree> *tuples, Index & idx);
	bool record_Condition(char * rec_bgn, int rec_Size, Condition & Conditions, vector<Attribute>& attrib_origs);
	int record_Output(vector<tuple_Entree>& tuples, vector<Attribute>& attribs, vector<Attribute>& attrib_origs);
	bool content_Fit_Conditions(char * content, type typ, Condition & Cdn);
	void content_Output(char * content, type typ);
};

#endif