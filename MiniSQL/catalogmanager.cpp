#include <iostream>
#include <fstream>
#include "catalogmanager.h"
#include "indexmanager.h"
#include "recordmanager.h"
#define NULL_NME "__NIL"

using namespace std;

CatalogManager::CatalogManager()
{
	int i, j;
	//Load from tableCatalog log file, save into memory as an entry of class Table
	ifstream ftable;
	ftable.open("tableCatalog.catalog");
	ftable >> tableNum;
	for (i = 0; i < tableNum; i++) {
		Table tmpT;
		ftable >> tmpT.name;
		ftable >> tmpT.attrNum;
		ftable >> tmpT.blockNum;
		tmpT.totalLength = 0;
		for (j = 0; j < tmpT.attrNum; j++) {
			Attribute tmpA;
			ftable >> tmpA.name;
			ftable >> tmpA.type;
			ftable >> tmpA.length;
			ftable >> tmpA.indexName;
			ftable >> tmpA.isPrimaryKey;
			ftable >> tmpA.isIndexed;
			ftable >> tmpA.isUnique;
			tmpT.totalLength += tmpA.length;
			tmpT.attributes.push_back(tmpA);
		}
		tables.push_back(tmpT);
	}

	//Load from indexCatalog and save into memory
	ifstream findex;
	findex.open("indexCatalog.catalog");
	findex >> indexNum;
	for (i = 0; i < indexNum; i++) {
		Index tmpI;
		findex >> tmpI.name;
		findex >> tmpI.tableName;
		findex >> tmpI.attrName;
		findex >> tmpI.typ;
		if (strcmp(tmpI.name.c_str(), NULL_NME) == 0)
		{
			tmpI.root = IndexManager::index_Read(index_File_GET(tmpI.tableName, tmpI.attrName));
			tmpI.name.clear();
		}
		else
			tmpI.root = IndexManager::index_Read(index_File_GET(tmpI.name));
		indices.push_back(tmpI);
	}
	ftable.close();
	findex.close();
}

CatalogManager::~CatalogManager()
{
	int i, j;
	//Save data in memory into file
	ofstream ftable;
	ftable.open("tableCatalog.catalog");
	ftable << tableNum << " ";
	for (i = 0; i < tableNum; i++) {
		ftable << tables[i].name << " ";
		ftable << tables[i].attrNum << " ";
		ftable << tables[i].blockNum << " ";
		for (j = 0; j < tables[i].attrNum; j++) {
			ftable << tables[i].attributes[j].name << " ";
			ftable << tables[i].attributes[j].type << " ";
			ftable << tables[i].attributes[j].length << " ";
			ftable << tables[i].attributes[j].indexName << " ";
			ftable << tables[i].attributes[j].isPrimaryKey << " ";
			ftable << tables[i].attributes[j].isIndexed << " ";
			ftable << tables[i].attributes[j].isUnique << " ";
		}
	}

	//Save index data into file
	ofstream findex;
	findex.open("indexCatalog.catalog");
	findex << indexNum << " ";
	for (i = 0; i < indexNum; i++) {
		if (indices[i].name.empty())
		{
			findex << NULL_NME << " ";
			IndexManager::index_Save(index_File_GET(indices[i].tableName, indices[i].attrName), indices[i].root);
		}
		else
		{
			findex << indices[i].name << " ";
			IndexManager::index_Save(index_File_GET(indices[i].name), indices[i].root);
		}
		findex << indices[i].tableName << " ";
		findex << indices[i].attrName << " ";
		findex << indices[i].typ << " ";

	}

	ftable.close();
	findex.close();
}

void CatalogManager::CreateTable(Table table)
{
	tableNum++;
	tables.push_back(table);
}

void CatalogManager::CreateIndex(Index index)
{
	indexNum++;
	indices.push_back(index);
	for (int i = 0; i < tableNum; i++) {
		if (tables[i].name == index.tableName) {
			for (int j = 0; j < tables[i].attrNum; j++) {
				if (tables[i].attributes[j].name == index.attrName) {
					tables[i].attributes[j].indexName = index.name;
					tables[i].attributes[j].isIndexed = true;
				}
			}
		}
	}
}

void CatalogManager::DropTable(string TName)
{
	int i;
	for (i = 0; i < tableNum; i++) {
		if (tables[i].name == TName) {
			tableNum--;
			tables.erase(tables.begin() + i);
			return;
		}
	}
}

void CatalogManager::index_Drop(string IName)
{
	int i;
	for (i = 0; i < indexNum; i++) {
		if (indices[i].name == IName) {
			indexNum--;
			indices.erase(indices.begin() + i);
			return;
		}
	}
}

void CatalogManager::AddTableBlockNum(string TName)
{
	int i;
	for (i = 0; i < tableNum; i++) {
		if (tables[i].name == TName) {

			tables[i].blockNum++;
			return;
		}
	}
}

bool CatalogManager::IsTable(string TName)
{
	int i;
	for (i = 0; i < tableNum; i++) {
		if (tables[i].name == TName) {
			return true;
		}
	}
	return false;
}

bool CatalogManager::IsIndex(string IName)
{
	int i;
	for (i = 0; i < indexNum; i++) {
		if (indices[i].name == IName) {
			return true;
		}
	}
	return false;
}

bool CatalogManager::IsAttribute(string TName, string AName)
{
	int i, j;
	for (i = 0; i < tableNum; i++) {
		if (tables[i].name == TName) {
			for (j = 0; j < tables[i].attrNum; j++) {
				if (tables[i].attributes[j].name == AName) {
					return true;
				}
			}
		}
	}
	return false;
}

Table& CatalogManager::GetTable(string TName)
{
	int i;
	for (i = 0; i < tableNum; i++) {
		if (tables[i].name == TName) {
			return tables[i];
		}
	}
}

Index& CatalogManager::GetIndex(string IName)
{
	int i;
	for (i = 0; i < indexNum; i++) {
		if (indices[i].name == IName) {
			return indices[i];
		}
	}
}

Index& CatalogManager::GetIndex(string tName, string attribName)
{
	int i;
	for (i = 0; i < indexNum; i++) {
		if (indices[i].tableName == tName && indices[i].attrName == attribName) {
			return indices[i];
		}
	}
}

bool CatalogManager::IndexOnTable(string TName, string IName)
{
	int i;
	for (i = 0; i < indexNum; i++) {
		if (indices[i].name == IName) {
			if (indices[i].tableName == TName) {
				return true;
			}
			else {
				return false;
			}
		}
	}
	return false;
}

bool CatalogManager::HasIndex(string TName, string AName)
{
	if (AName == "*")
	{
		for (int i = 0; i < indexNum; i++)
		if (indices[i].tableName == TName) return true;
		return false;
	}
	for (int i = 0; i < indexNum; i++)
	{
		if (indices[i].tableName == TName && indices[i].attrName == AName)
		{
			return true;
		}
	}
	return false;
}

