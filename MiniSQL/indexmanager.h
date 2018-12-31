#pragma once
#include <cstdio>
#include <string>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include "buffermanager.h"
#include "catalogmanager.h"
#include "recordmanager.h"
#include "Public.h"
#define N_TREE 5

using namespace std;
class BufferManager;
class RecordManager;
class CatalogManager;
class Attribute;
class Table;
class Index;

class Node;
class datum {
public:
	int block_no;
	int tuple_no;
	datum(int a, int b) :block_no(a), tuple_no(b) {}
	datum() {};
	~datum() {};
};

class Node {
public:
	int node_type;
	vector<string> key;
	Node* ptr_father;
	int key_type;
	Node() { ptr_father = NULL; }
	virtual ~Node() {
			
	};
	Node(Node* father, int type, int value_type) :
		ptr_father(father), node_type(type), key_type(value_type) {};
	string find_min();
	string find_max();
};

class NormalNode :public Node {
public:
	vector<Node* > ptr_child; //注意使用的时候需要动态类型转换
	NormalNode* left_sibling;
	NormalNode* right_sibling;
	NormalNode() {
		this->node_type = 0;
	};
	~NormalNode() {};
	NormalNode(NormalNode* father, NormalNode* l_sibling, NormalNode* r_sibling, int type, int value_type) :
		left_sibling(l_sibling), right_sibling(r_sibling) {
		this->ptr_father = father;
		this->node_type = type;
		this->key_type = value_type;
	}

	void node_Insert(string temp_key, datum block_index, Node* &root);
	void branch_Break(Node* &root);
	void normalnode_Delete(string temp_key,Node* &root);
	void node_Alter(Node* &root);
	void borrow_from(NormalNode* temp_node);
	void leaf_Merge(NormalNode* temp_node, Node* & root);
};

class Leaf :public Node {
public:
	Leaf* left_sibling;
	Leaf* right_sibling;
	vector<datum> ptr_data;
	Leaf() { this->node_type = 1; }
	~Leaf() {};
	Leaf(NormalNode* father, Leaf* l_sibling, Leaf* r_sibling, int type, int value_type) :
		left_sibling(l_sibling), right_sibling(r_sibling) {
		this->ptr_father = father;
		this->node_type = type;
		this->key_type = value_type;
	}

	void node_Insert(string temp_key, datum block_num, Node* &root);
	void leaf_Break(Node* &root);
	void leaf_Delete(string temp_key, Node* & root);
	void borrow_from(Leaf* temp_node);
	void leaf_Merge(Leaf* temp_node, Node* &root);
};

class IndexManager
{
public:
	IndexManager() {}
	IndexManager(BufferManager *BM, CatalogManager *CM)
	{
		bm = BM;
		cm = CM;
	}
	~IndexManager() {}
	BufferManager *bm;
	CatalogManager *cm;
	static void index_Save(string filename, Node* &root);
	static Node* index_Read(string filename);
	void index_Drop(string filename, Node* root);
	datum select_OnEqual(string key, Node* root);
	void select_InBetween(vector<datum>& result_data, string start_key, string end_key, Node* root, int start_flag, int end_flag);
	static int insert_pos(vector<string>&key, string temp_key, int start, int end, int value_type);
	bool index_Drop(string index_name);
	string key_GET(char * tuple, vector<Attribute> attributes, string attr_name);
	void select_InBetween(string table_name, vector<char*>& result_data, string start_key, string end_key, Node* root, int start_flag, int end_flag);
	vector<tuple_Entree> select_InBetween(string table_name, string start_key, string end_key, Node * root, int start_flag, int end_flag);
	static Leaf * leftleaf_GET(Node * root);
	tuple_Entree select_OnEqual(string table_name, string temp_key, Node * root);
	void index_Update(int update_type,Table* table_ptr,tuple_Entree temp_data);
	bool CreateIndex(string index_name, Table * table_ptr, string attr_name, type typ);
	vector<tuple_Entree> IndexManager::select_OnNotEqual(string table_name, string temp_key0, Node* root);
};


