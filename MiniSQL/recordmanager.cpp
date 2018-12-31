//
// recordmanager.cpp
// @ minisql - 2017D - Database Management Systems by Zhuang Yueting
//
// Created by 陈潼 on 4-6-17; Copyright (c) 2017 Tony Chen. All rights reserved
//

#include <iostream>
#include <string>
#include <cstring>
#include "recordmanager.h"
#include "public.h"
#include "catalogmanager.h"
#include "indexmanager.h"

#define __GENERAL_ATTR "*"

int RecordManager::table_Create(string name)
{
	string table = table_File_GET(name);
	FILE *fp = fopen(table.c_str(), "w+");
	if (!fp) return -1;
	fclose(fp);
	return 0;
}

int RecordManager::table_Drop(string name)
{
	int q = bm->table_Drop(name);
	if (q < 0) return q;
	cm->DropTable(name);
	system((string("rm ") + table_File_GET(name)).c_str());
	return q;
}

int RecordManager::index_Create(string name, string attrib, string table_name)
{
	type t = attrib_this_GET(table_name, attrib);
	string u = name;
	if (name.empty())  u = index_File_GET(table_name, attrib);
	Table tn = cm->GetTable(table_name);
	im->CreateIndex(name, &tn, attrib, t);
	FILE *fp = fopen(u.c_str(), "w+");
	if (!fp) return -1;
	fclose(fp);
	return 0;
}

int RecordManager::index_Drop(string name, string attrib, string table_name)
{
	string index = name;
	if (name.empty()) index = index_File_GET(name, attrib);
	im->index_Drop(index);
	return 0;
}

int RecordManager::record_Insert(string table, vector<string>* rec, vector<type> *typ)
{
	string t, tmp;
	bool has_Prim = 0;
	tuple_Entree *te;
	int size_str = 0;
	vector<Attribute> attribs = attrib_GET(table);
	Condition cd;
	for (int i = 0; i < attribs.size(); i++)
	{
		if (attribs[i].isPrimaryKey || attribs[i].isUnique)
		{
			has_Prim = 1;
			cd.attrib_Name.push_back(attribs[i].name);
			cd.attrib_Type.push_back(attribs[i].type);
			cd.attrib_Value.push_back((*rec)[i]);
			cd.attrib_Operator.push_back(Condition::optr_EQUAL);
			cd.attrib_Relation.push_back(__and);
		}
	}
	for (unsigned int k = 0; k < rec->size(); k++)
	{
		switch ((*typ)[k])
		{
		case __int:
			tmp = itob(atoi((*rec)[k].c_str()));
			tmp[__INT_SIZE] = 32;
			break;
		case __float:
			tmp = ftob(atof((*rec)[k].c_str()));
			tmp[__FLOAT_SIZE] = 32;
			break;
		case __string:
			tmp = (*rec)[k];
			while (tmp.length() < attribs[k].length) tmp = tmp + string(" ");
			//tmp[attribs[k].length] = '\0';
			break;
		}
		t = t + tmp + " ";
		size_str += attribs[k].length + 1;
	}
	t = t + string("\0");
	if (record_Find(table, cd)->size()>0) return -1;
	te = bm->record_Insert(table, (char*)t.c_str(), size_str);
	if (te->block_id == -1) return -1;
	if (cm->HasIndex(table, __GENERAL_ATTR))
	{
		im->index_Update(0, &cm->GetTable(table), *te);
	}
	return 0;
}

int RecordManager::record_Show(string table, vector<string>* attrib_list, Condition &conditions)
{
	vector<Attribute> attr, attr_orig;
	if ((*attrib_list)[0] == "*") attr = attrib_GET(table);
	else attr = attrib_Convert(table, attrib_list);
	attr_orig = attrib_GET(table);
	vector<tuple_Entree> *tuples = record_Find(table, conditions);
	record_Output(*tuples, attr, attr_orig);
	return tuples->size();
}

vector<tuple_Entree> *RecordManager::record_Find(string table, Condition &conditions)
{
	vector<tuple_Entree> *tuples = new vector <tuple_Entree>;
	if (conditions.attrib_Name[0] == "*")
	{
		int u = 0, table_Num = cm->GetTable(table).blockNum;
		block *b;
		int n_This_Block;
		while (u < table_Num)
		{
			b = bm->block_GET(table, u);
			n_This_Block = b->getRecordCount();
			for (int i = 1; i <= n_This_Block; i++)
			{
				int pos = b->getRecordPosition(i);
				int siz = b->getRecordSize(i);
				if (siz == 0) continue;
				char *s = (char*)malloc(siz + 1);
				strncpy(s, b->record_GET(i), siz);
				s[siz] = 0;
				tuple_Entree *t = new tuple_Entree();
				t->block_id = u;
				t->entry_Tuple = s;
				t->id = i;
				tuples->push_back(*t);
			}
			u++;
		}
		return tuples;
	}
	for (unsigned int i = 0; i < conditions.attrib_Name.size(); i++)
	{
		Condition cd(conditions.attrib_Name[i], conditions.attrib_Type[i], conditions.attrib_Value[i], conditions.attrib_Operator[i], ((i>0) ? conditions.attrib_Relation[i - 1] : __NaO));
		tuples = record_Find(table, cd, tuples);
	}
	return tuples;
}

vector<tuple_Entree> *RecordManager::record_Find(string table, Condition & conditions, vector<tuple_Entree> *tuples)
{
	vector<Attribute> attribs = cm->GetTable(table).attributes;
	
	int recSize = 0;
	for (int i = 0; i < attribs.size(); i++)
	{
		recSize += (int)attribs[i].length;
		recSize++;
		if (conditions.attrib_Name[0] == attribs[i].name)
		{
			while (conditions.attrib_Value[0].length() < attribs[i].length)  conditions.attrib_Value[0] += " ";
			conditions.attrib_Value[0] += "\0";
		}
	}
	vector<tuple_Entree>::iterator itr, tmp;
	char content[400];
	switch (conditions.attrib_Relation[0])
	{
	case __and:
		for (itr = tuples->begin(); itr != tuples->end();)
		{
			if (!record_Condition(itr->entry_Tuple, recSize, conditions, attribs))
			{
				tmp = itr;
				tmp->entry_Tuple[recSize] = 0;
				itr = tuples->erase(tmp);
			}
			else itr++;
		}
		break;
	case __or:
	case __NaO:
		if (cm->HasIndex(table, conditions.attrib_Name[0]))
			return record_Find_w_Index(table, conditions, tuples, cm->GetIndex(table, conditions.attrib_Name[0]));
		int u = 0, table_Num = cm->GetTable(table).blockNum;
		block *b;
		int n_This_Block;

		while (u < table_Num)
		{
			b = bm->block_GET(table, u);
			n_This_Block = b->getRecordCount();
			for (int i = 1; i <= n_This_Block; i++)
			{
				int pos = b->getRecordPosition(i);
				int siz = b->getRecordSize(i);
				if (siz == 0) continue;
				strncpy(content, b->record_GET(i), siz);
				content[siz] = 0;
				if (record_Condition(content, siz, conditions, attribs))
				{
					tuple_Entree *t = new tuple_Entree();
					t->block_id = u;
					t->entry_Tuple = (char*)malloc(siz+1);
					strncpy(t->entry_Tuple, content, siz);
					t->entry_Tuple[siz] = 0;
					t->id = i;
					tuples->reserve(tuples->size() + 5);
					tuples->push_back(*t);
				}
			}
			u++;
		}
		break;
	}
	vector<tuple_Entree>::iterator iter = unique(tuples->begin(), tuples->end());;
	while (iter != tuples->end()) {
		tuples->erase(iter, tuples->end());
	}
		return tuples;
}

vector<tuple_Entree> *RecordManager::record_Find_w_Index(string table, Condition & conditions, vector<tuple_Entree> *tuples, Index & idx)
{
	vector<tuple_Entree> tuple_Get_New;
	Node* index_node = idx.root;
	switch (conditions.attrib_Operator[0])
	{
	case Condition::optr_EQUAL:
		tuples->push_back(im->select_OnEqual(table, conditions.attrib_Value[0], index_node));
		break;
	case Condition::optr_MORE:
		tuple_Get_New = im->select_InBetween(table, conditions.attrib_Value[0], index_node->find_max(), index_node, 0, 1);
		break;
	case Condition::optr_MORE_EQUAL:
		tuple_Get_New = im->select_InBetween(table, conditions.attrib_Value[0], index_node->find_max(), index_node, 1, 1);
		break;
	case Condition::optr_LESS:
		tuple_Get_New = im->select_InBetween(table, index_node->find_min(), conditions.attrib_Value[0], index_node, 1, 0);
		break;
	case Condition::optr_LESS_EQUAL:
		tuple_Get_New = im->select_InBetween(table, index_node->find_min(), conditions.attrib_Value[0], index_node, 1, 1); 
		break;
	}
	for (unsigned int i = 0; i < tuple_Get_New.size(); i++)
	{
		tuples->push_back(tuple_Get_New[i]);
	}
	vector<tuple_Entree>::iterator iter = unique(tuples->begin(), tuples->end());;
	while (iter != tuples->end()) {
		tuples->erase(iter, tuples->end());
	}
	return tuples;
}

int RecordManager::record_Delete(string table, Condition &conditions)
{
	vector<tuple_Entree> *tuples = record_Find(table, conditions);
	for (unsigned int i = 0; i < tuples->size(); i++)
	{
		bm->record_Delete((*tuples)[i].block_id, (*tuples)[i].id);
		if (cm->HasIndex(table, __GENERAL_ATTR))
		{
			im->index_Update(1, &cm->GetTable(table), (*tuples)[i]);
		}
	}
	return tuples->size();
}

int RecordManager::record_Output(vector<tuple_Entree> &tuples, vector<Attribute> &attribs, vector<Attribute> &attrib_origs)
{
	char t[300];
	for (unsigned int i = 0; i < tuples.size(); i++)
	{
		int stt = 0, ende = 0, tmp;
		for (unsigned int j = 0; j < attrib_origs.size(); j++)
		{
			tmp = attrib_origs[j].length;
			ende += tmp + 1;
			strncpy(t, tuples[i].entry_Tuple + stt, tmp);
			t[tmp] = '\0';
			for (unsigned int k = 0; k < attribs.size(); k++)
				if (attrib_origs[j].name == attribs[k].name)
				{
					content_Output(t, attribs[k].type);
					
					break;
				}
			stt = ende;
		}
		cout << endl;
	}
	return tuples.size();
}

bool RecordManager::content_Fit_Conditions(char * content, type typ, Condition &Cdn)
{
	if (typ == __int)
	{
		int tmp2 = atoi(Cdn.attrib_Value[0].c_str());
		int tmp = btoi(content);   //get content value by point
		switch (Cdn.attrib_Operator[0])
		{
		case Condition::optr_EQUAL:
			return tmp == tmp2;
			break;
		case Condition::optr_NOT_EQUAL:
			return tmp != tmp2;
			break;
		case Condition::optr_LESS:
			return tmp < tmp2;
			break;
		case Condition::optr_MORE:
			return tmp > tmp2;
			break;
		case Condition::optr_LESS_EQUAL:
			return tmp <= tmp2;
			break;
		case Condition::optr_MORE_EQUAL:
			return tmp >= tmp2;
			break;
		default:
			return true;
			break;
		}
	}
	else if (typ == __float)
	{
		double tmp2 = atof(Cdn.attrib_Value[0].c_str());
		double tmp = btof(content);   //get content value by point
		switch (Cdn.attrib_Operator[0])
		{
		case Condition::optr_EQUAL:
			return tmp == tmp2;
			break;
		case Condition::optr_NOT_EQUAL:
			return tmp != tmp2;
			break;
		case Condition::optr_LESS:
			return tmp < tmp2;
			break;
		case Condition::optr_MORE:
			return tmp > tmp2;
			break;
		case Condition::optr_LESS_EQUAL:
			return tmp <= tmp2;
			break;
		case Condition::optr_MORE_EQUAL:
			return tmp >= tmp2;
			break;
		default:
			return true;
			break;
		}
	}
	else
	{
		string tmp2 = Cdn.attrib_Value[0];
		string tmp = content;
		switch (Cdn.attrib_Operator[0])
		{
		case Condition::optr_EQUAL:
			return tmp == tmp2;
			break;
		case Condition::optr_NOT_EQUAL:
			return tmp != tmp2;
			break;
		case Condition::optr_LESS:
			return tmp < tmp2;
			break;
		case Condition::optr_MORE:
			return tmp > tmp2;
			break;
		case Condition::optr_LESS_EQUAL:
			return tmp <= tmp2;
			break;
		case Condition::optr_MORE_EQUAL:
			return tmp >= tmp2;
			break;
		default:
			return true;
			break;
		}
	}
}

void RecordManager::content_Output(char * content, type typ)
{
	if (typ == __int)
	{
		//if the content is a int
		int tmp = btoi(content);   //get content value by point
		printf("%d ", tmp);
	}
	else if (typ == __float)
	{
		//if the content is a float
		double tmp = btof(content);   //get content value by point
		printf("%f ", tmp);
	}
	else
	{
		//if the content is a string
		string tmp = content;
		printf("%s ", tmp.c_str());
	}
}

vector<Attribute> RecordManager::attrib_GET(string table)
{
	Table &t = cm->GetTable(table);
	return t.attributes;
}

vector<Attribute> RecordManager::attrib_Convert(string table, vector<string>* attrib_list)
{
	vector<Attribute> Goal, Ref = cm->GetTable(table).attributes;
	for (unsigned int i = 0; i < attrib_list->size(); i++)
	{
		for (unsigned int j = 0; j < Ref.size(); j++)
		{
			if (Ref[j].name == (*attrib_list)[i])
			{
				Goal.push_back(Ref[j]);
			}
		}
	}
	return Goal;
}

type RecordManager::attrib_this_GET(string table, string attrib)
{
	vector <Attribute> v = attrib_GET(table);
	for (unsigned int i = 0; i < v.size(); i++)
	{
		if (attrib == v[i].name) return v[i].type;
	}
}

int RecordManager::table_Size_GET(string TName)
{
	int u = cm->GetTable(TName).blockNum;
	int res = 0;
	for (int i = 0; i < u; i++)
	{
		res += bm->block_GET(TName, i)->getRecordCount();
	}
	return res;
}

bool RecordManager::record_Condition(char* rec_bgn, int rec_Size, Condition &Conditions, vector<Attribute>& attrib_origs)
{
	type typ;
	string attributeName;
	int typeSize;
	char content[255];
	bool result = true;
	char *contentBegin = rec_bgn;
	for (unsigned int i = 0; i < attrib_origs.size(); i++)
	{
		typ = attrib_origs[i].type;
		attributeName = attrib_origs[i].name;
		typeSize = attrib_origs[i].length;
		//init content (when content is string , we can get a string easily)
		memcpy(content, contentBegin, typeSize);
		content[typeSize] = 0;
		if (Conditions.attrib_Name[0] == "*")
		{
			return true;
		}
		if (Conditions.attrib_Name[0] == attributeName)
		{
			return content_Fit_Conditions(content, Conditions.attrib_Type[0], Conditions);
		}
		contentBegin += typeSize + 1;
	}
	return false;
}

/*
int RecordManager::record_Delete(string table, Condition &conditions, block * Blk)
{
	int count = 0, u = Blk->getRecordCount(), blk_no = Blk->getNum();
	char* recordBegin = Blk->blockHeadPointer_GET();
	vector<Attribute> attrib_origs = attrib_GET(table);
	char* blockBegin = Blk->record_GET(0);
	int recSize = Blk->getRecordSize(0);
	for (unsigned int i = 0; i < u; i++)
	{
		recordBegin = Blk->record_GET(i);
		if (record_Condition(recordBegin, recSize, conditions, attrib_origs))
		{
			count++;
			bm->record_Delete(blk_no, i);
		}
	}
	return count;
}

int RecordManager::record_Show(string table, vector<Attribute> attribs, Condition &conditions, block *Blk)
{
	int count = 0, u = Blk->getRecordCount();
	char* recordBegin = Blk->blockHeadPointer_GET();
	vector<Attribute> attrib_origs = attrib_GET(table);
	char* blockBegin = Blk->record_GET(0);
	int recSize = Blk->getRecordSize(0);
	for (unsigned int i = 0; i < u; i++)
	{
		recordBegin = Blk->record_GET(i);
		if (record_Condition(recordBegin, recSize, conditions, attrib_origs))
		{
			count++;
			record_Output(recordBegin, recSize, attribs, attrib_origs);
		}
	}
	return count;
}



int RecordManager::record_Show(string table, vector<string>* attrib_list, Condition &conditions)
{
	block *b = bm->block_GET(table, 0);
	int u = 0;
	int count = 0;
	bool checked = 0;
	int recordBlockNum;
	vector<Attribute> attr;
	if ((*attrib_list)[0] == "*") attr = attrib_GET(table);
	else vector<Attribute> attr = attrib_Convert(table, attrib_list);
	vector<char*> Entry;
	while (true)
	{
		if (b == NULL)
		{
			if (!checked) return -1;
			return count;
		}
		checked = 1;
		recordBlockNum = record_Show(table, attr, conditions, b);
		count += recordBlockNum;
		u++;
		b = bm->block_GET(table, u);
	}
	//TODO: 从条目优先改为条件优先
}

int RecordManager::record_Delete(string table, Condition &conditions)
{
	block *b = bm->block_GET(table, 0);
	int u = 0;
	int count = 0;
	bool checked = 0;
	int recordBlockNum;
	while (true)
	{
		if (b == NULL)
		{
			if (!checked) return -1;
			return count;
		}
		checked = 1;
		recordBlockNum = record_Delete(table, conditions, b);
		count += recordBlockNum;
		u++;
		b = bm->block_GET(table, u);
	}
	return -1;
}
*/