#pragma once
#include <string>
#include <vector>
#include "buffermanager.h"
#include "catalogmanager.h"
#include "recordmanager.h"
#include "indexmanager.h"
#include "Public.h"
using namespace std;

class BufferManager;
class RecordManager;
class CatalogManager;
class IndexManager;
enum type;

class Interpreter
{
public:
	BufferManager *bm;
	CatalogManager *cm;
	RecordManager *rm;
	IndexManager *im;
	string filename;
	Interpreter();
	~Interpreter(){}
	int interpreter(string s);
	string getword(string s, int *st);
	type judge_type(string s);
};

