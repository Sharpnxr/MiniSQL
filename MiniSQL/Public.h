#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;
enum type { __int, __float, __string };
enum oprtr { __NaO, __and, __or, __not_and, __not_or };
#define __INT_SIZE 8
#define __FLOAT_SIZE 16

class Node;

class Attribute {
public:
	string name;
	type type;
	int length;
	string indexName; //与该属性相关的index名称
	bool isPrimaryKey;
	bool isIndexed;
	bool isUnique;
};

class Table {
public:
	//表的基本信息
	string name;
	int attrNum;
	vector <Attribute> attributes;
	//表的数据块信息
	int blockNum;//该表使用的数据块的个数	
	int totalLength;
};

class Index {
public:
	string name;
	string tableName;
	string attrName;
	Node* root;
	type typ;
	Index(string temp_name, string temp_table, string temp_arr, Node* temp_root, type temp_type) :
		name(temp_name), tableName(temp_table), attrName(temp_arr), root(temp_root), typ(temp_type) {}
	Index()
	{
		name = "/0";
		tableName = "/0";
		attrName = "/0";
		root = 0;
		typ = __int;
	}
	~Index() {};

};

class Condition
{
public:
	const static int optr_EQUAL = 0; // "="
	const static int optr_NOT_EQUAL = 1; // "<>"
	const static int optr_LESS = 2; // "<"
	const static int optr_MORE = 3; // ">"
	const static int optr_LESS_EQUAL = 4; // "<="
	const static int optr_MORE_EQUAL = 5; // ">="

	vector <string> attrib_Name;
	vector <type> attrib_Type;
	vector <string> attrib_Value;
	vector <int> attrib_Operator;
	vector <oprtr> attrib_Relation;// the value to be compared

	Condition(string attrib_nme, type attrib_typ, string attrib_val, int attrib_oprtr, oprtr attrib_rel)
	{
		attrib_Name.push_back(attrib_nme);
		attrib_Type.push_back(attrib_typ);
		/*string *temp;
		temp = new string(attrib_val);*/
		attrib_Value.push_back(attrib_val);
		attrib_Operator.push_back(attrib_oprtr);
		attrib_Relation.push_back(attrib_rel);
	}

	Condition() {}
	Condition(int i)
	{
		if (i == -1)
		{
			attrib_Name.push_back("*");
			attrib_Type.push_back(__string);
			attrib_Value.push_back("/0");
			attrib_Operator.push_back(17);
			attrib_Relation.push_back(__NaO);
		}
	}
};

typedef struct
{
	char *entry_Tuple;
	int id;
	int block_id;
} tuple_Entree;

int btoi(const char *s);
double btof(const char *s);
char *itob(const int s);
char *ftob(const double s);

string index_File_GET(string name);
string index_File_GET(string t_name, string attrib);
string table_File_GET(string name);

ofstream &operator<<(ofstream &fout, bool &t);
ifstream &operator>>(ifstream &fin, bool &t);
ofstream &operator<<(ofstream &fout, type &t);
ifstream &operator>>(ifstream &fin, type &t);
bool operator ==(const tuple_Entree & q, const tuple_Entree & p);