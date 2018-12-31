#pragma once
#include <vector>
#include "buffermanager.h"
#include "indexmanager.h"
#include "Interpreter.h"
#include "recordmanager.h"
#include "Public.h"

class BufferManager;
class RecordManager;
class IndexManager;

class CatalogManager {
public:
	int tableNum;
	int indexNum;
	vector <Table> tables;
	vector <Index> indices;

	CatalogManager();
	~CatalogManager();
	void CreateTable(Table table);
	void CreateIndex(Index index);
	void DropTable(string TName);
	void index_Drop(string IName);
	void AddTableBlockNum(string TName);
	bool IsTable(string TName);
	bool IsIndex(string IName);
	bool IsAttribute(string TName, string AName);
	Table& GetTable(string TName);
	Index& GetIndex(string IName);
	Index& GetIndex(string tName, string attribName);
	bool IndexOnTable(string TName, string IName);
	bool HasIndex(string TName, string AName);
};