#include <vector>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <cstdio>
#include <fstream> 
#include "indexmanager.h"
#include <stdio.h>
#define N_TREE 5
#define N_HALF_TREE ((N_TREE+1)/2)

using namespace std;

void NormalNode::node_Insert(string temp_key, datum block_index, Node* &root) {
	int pos = IndexManager::insert_pos(this->key, temp_key, 0, this->key.size(), this->key_type);
	if (ptr_child[pos]->node_type)
		((Leaf*)ptr_child[pos])->node_Insert(temp_key, block_index, root);
	else
		((NormalNode*)ptr_child[pos])->node_Insert(temp_key, block_index, root);
	return;
}

void Leaf::node_Insert(string temp_key, datum block_num, Node* &root) {
	int pos;

	if (this->key.size() == 0) {
		this->key.push_back(temp_key);
		this->ptr_data.push_back(block_num);
	}
	else {
		pos = IndexManager::insert_pos(this->key, temp_key, 0,this->key.size(), this->key_type);
		this->key.insert(this->key.begin() + pos, temp_key);
		this->ptr_data.insert(this->ptr_data.begin() + pos, block_num);
	}
	if (key.size() <= N_TREE)
		return;
	else {
		this->leaf_Break(root);
	}
	return;
}

//叶子分解函数
void Leaf::leaf_Break(Node* &root) {
	int key_num = this->key.size();
	Leaf* temp_leaf = new Leaf();
	temp_leaf->key_type = root->key_type;

	NormalNode* temp_branch;

	for (int i = key_num / 2; i < key_num; i++) {
		temp_leaf->key.push_back(this->key[i]);
		temp_leaf->ptr_data.push_back(this->ptr_data[i]);
	}

	this->key.erase((this->key.begin() + key_num / 2), this->key.end());
	this->ptr_data.erase((this->ptr_data.begin() + key_num / 2), this->ptr_data.end());

	if (this->right_sibling != NULL)
		temp_leaf->right_sibling = this->right_sibling;

	this->right_sibling = temp_leaf;
	temp_leaf->left_sibling = this;

	//如果该叶子即是根则分解叶子后，新建一个父节点为根
	if (this->ptr_father == NULL) {
		temp_branch = new NormalNode();
		temp_branch->key_type = root->key_type;
		temp_branch->key.push_back(temp_leaf->key[0]);
		temp_branch->ptr_child.push_back((Leaf*)this);
		temp_branch->ptr_child.push_back(temp_leaf);
		root = temp_branch;
		root->node_type = 0;
		//Modify 6.19
		this->ptr_father = temp_branch;
		temp_leaf->ptr_father = temp_branch;
		//Modify 6.19
	}
	//否则对父节点进行处理
	else {
		NormalNode* temp_father = (NormalNode*)(this->ptr_father);
		int temp_pos;
		//判断原来的叶子的第一个值是否出现在父亲节点中
		if (find(temp_father->key.begin(), temp_father->key.end(), this->key[0]) != temp_father->key.end())
			temp_pos = find(temp_father->key.begin(), temp_father->key.end(), this->key[0]) - temp_father->key.begin();
		//Modify 6.19
		//else if ((Leaf*)(temp_father->ptr_child[0]) != this)
			//temp_pos = IndexManager::insert_pos(temp_father->key, this->key[0], 0, this->key.size(), temp_father->key_type);
		else
			temp_pos = -1;

		temp_father->key.insert((temp_father->key.begin()+1+temp_pos), temp_leaf->key[0]);
		temp_father->ptr_child.insert((temp_father->ptr_child.begin() +2+temp_pos), temp_leaf);
		temp_leaf->ptr_father = this->ptr_father;

		if (temp_father->ptr_child.size() > N_TREE)
			temp_father->branch_Break(root);

		return;
	}
}

void NormalNode::branch_Break(Node* &root) {
	int child_num = this->ptr_child.size();
	NormalNode* temp_branch = new NormalNode();
	temp_branch->key_type = root->key_type;
	NormalNode* temp_root;

	for (int i = child_num / 2; i < child_num; i++)
		temp_branch->ptr_child.push_back(ptr_child[i]);

	ptr_child.erase(ptr_child.begin() + child_num / 2, ptr_child.end());
	this->key.clear();

	if (this->right_sibling != NULL)
		temp_branch->right_sibling = this->right_sibling;

	this->right_sibling = temp_branch;
	temp_branch->left_sibling = this;

	for (unsigned int i = 1; i < this->ptr_child.size(); i++)
		this->key.push_back(this->ptr_child[i]->find_min());

	for (unsigned int i = 1; i < temp_branch->ptr_child.size(); i++)
		temp_branch->key.push_back(temp_branch->ptr_child[i]->find_min());
	for (unsigned int i = 0; i < temp_branch->ptr_child.size();i++)
		temp_branch->ptr_child[i]->ptr_father = temp_branch;
	if (this->ptr_father == NULL) {
		temp_root = new NormalNode();
		temp_root->key_type = root->key_type;
		temp_root->key.push_back(temp_branch->find_min());
		temp_root->ptr_child.push_back(this);
		temp_root->ptr_child.push_back(temp_branch);
		root = temp_root;
		root->node_type = 0;
		//Update 6.19
		this->ptr_father = root;
		temp_branch->ptr_father = root;
		//Update 6.19
	}
	
	else {
		NormalNode* temp_father = (NormalNode*)(this->ptr_father);
		int temp_pos;
		if (find(temp_father->key.begin(), temp_father->key.end(), this->find_min()) != temp_father->key.end())
			temp_pos = find(temp_father->key.begin(), temp_father->key.end(), this->find_min()) - temp_father->key.begin();
		//Update 6.19
		else if ((NormalNode*)(temp_father->ptr_child[0]) != this)
			temp_pos = IndexManager::insert_pos(temp_father->key, this->find_min(), 0, this->key.size(), temp_father->key_type);
		else
			temp_pos = -1;

		temp_father->key.insert(temp_father->key.begin() + 1 + temp_pos , temp_branch->find_min());
		temp_father->ptr_child.insert(temp_father->ptr_child.begin() + 2+ temp_pos, temp_branch);

		temp_branch->ptr_father = this->ptr_father;

		if (temp_father->ptr_child.size() > N_TREE)
			temp_father->branch_Break(root);
	}
	return;
}

void Leaf::leaf_Delete(string temp_key, Node* & root) {
	Leaf* right;
	Leaf* left;
	if (this->key.size() == 0) {
		printf("Error:It's an empty table!\n");
		return;
	}

	vector<string>::iterator temp_it = find(this->key.begin(), this->key.end(), temp_key);
	if (temp_it != this->key.end()) {
		int temp_pos = temp_it - this->key.begin();
		this->key.erase(temp_it);
		this->ptr_data.erase(this->ptr_data.begin() + temp_pos);
		//Update 6.19
		NormalNode* temp_father = (NormalNode*)this->ptr_father;
		if (find(temp_father->key.begin(), temp_father->key.end(), temp_key) != temp_father->key.end())
			*find(temp_father->key.begin(), temp_father->key.end(), temp_key) = this->key[0];
		else if (this->left_sibling != NULL&&this->left_sibling->ptr_father != this->ptr_father) {
			temp_father = (NormalNode*)temp_father->ptr_father;
			if (find(temp_father->key.begin(), temp_father->key.end(), temp_key) != temp_father->key.end())
				*find(temp_father->key.begin(), temp_father->key.end(), temp_key) = this->key[0];
		}
		//Update 6.19
	}
	else {
		cout << "There didn't have value " << temp_key << endl;
		return;
	}

	if (this->ptr_father == NULL)
		return;
	else if (this->key.size() < N_HALF_TREE) {
		right = this->right_sibling;
		left = this->left_sibling;
		if (right == NULL) {
			if (left->key.size() > N_HALF_TREE)
				this->borrow_from(left);
			else
				this->leaf_Merge(left, root);
		}
		else if (left == NULL) {
			if (right->key.size() > N_HALF_TREE)
				this->borrow_from(right);
			else
				this->leaf_Merge(right, root);
		}
		else {
			if (left->ptr_father != this->ptr_father)
				if (right->key.size() > N_HALF_TREE)
					this->borrow_from(right);
				else
					this->leaf_Merge(right, root);
			else if (right->ptr_father != this->ptr_father)
				if (left->key.size() > N_HALF_TREE)
					this->borrow_from(left);
				else
					this->leaf_Merge(left, root);
			else {
				if (left->key.size() == N_HALF_TREE&&right->key.size() == N_HALF_TREE)
					this->leaf_Merge(right, root);
				else if (left->key.size() == N_HALF_TREE&&right->key.size() > N_HALF_TREE)
					this->borrow_from(right);
				else if (right->key.size() == N_HALF_TREE&&left->key.size() > N_HALF_TREE)
					this->borrow_from(left);
				else
					this->borrow_from((left->key.size() > right->key.size()) ? left : right);
			}
		}
	}
	else
		return;

	return;
}

void NormalNode::normalnode_Delete(string temp_key, Node* &root) {
	int pos = IndexManager::insert_pos(this->key, temp_key, 0, this->key.size(), this->key_type);
	if (ptr_child[pos]->node_type)
		((Leaf*)ptr_child[pos])->leaf_Delete(temp_key, root);
	else
		((NormalNode*)ptr_child[pos])->normalnode_Delete(temp_key, root);
	return;
}

void Leaf::borrow_from(Leaf* temp_node) {
	NormalNode* temp_father = (NormalNode*)(this->ptr_father);
	if (temp_node == this->left_sibling) {
		this->key.insert(this->key.begin(), *(temp_node->key.end() - 1));
		temp_node->key.pop_back();
		this->ptr_data.insert(this->ptr_data.begin(), *(temp_node->ptr_data.end() - 1));
		temp_node->ptr_data.pop_back();
		*(find(temp_father->key.begin(), temp_father->key.end(), this->key[1])) = this->key[0];
	}
	else {
		//Update 6.19
		*(find(temp_father->key.begin(), temp_father->key.end(), temp_node->key[0])) = temp_node->key[1];
		//Update 6.19
		this->key.push_back(temp_node->key[0]);
		this->ptr_data.push_back(temp_node->ptr_data[0]);
		temp_node->key.erase(temp_node->key.begin());
		temp_node->ptr_data.erase(temp_node->ptr_data.begin());
	}
	return;
}

void Leaf::leaf_Merge(Leaf* temp_node, Node* &root) {
	int temp_pos;
	NormalNode* temp_father = (NormalNode*)(this->ptr_father);
	Leaf* temp_left, *temp_right;

	if (temp_node == this->left_sibling) {
		temp_left = temp_node;
		temp_right = this;
	}
	else {
		temp_left = this;
		temp_right = temp_node;
	}

	temp_pos = find(temp_father->key.begin(), temp_father->key.end(), temp_right->key[0]) - temp_father->key.begin();
	for (unsigned int i = 0; i < temp_right->key.size(); i++) {
		temp_left->key.push_back(temp_right->key[i]);
		temp_left->ptr_data.push_back(temp_right->ptr_data[i]);
	}
	temp_father->key.erase(temp_father->key.begin() + temp_pos);
	temp_father->ptr_child.erase(temp_father->ptr_child.begin() + temp_pos + 1);
	temp_left->right_sibling = temp_right->right_sibling;

	delete temp_right;

	if (temp_father->ptr_child.size() < N_HALF_TREE)
		if (temp_father->ptr_father != NULL)
			temp_father->node_Alter(root);
		else if (temp_father->ptr_child.size() == 1) {
			root = temp_left;
			root->ptr_father = NULL;
			delete temp_father;
		}
		else
			return;
	return;
}

//the num of branch'childs isn't satisfied,need to borrow or merge
void NormalNode::node_Alter(Node* &root) {
	NormalNode* temp_father = (NormalNode*)(this->ptr_father);
	NormalNode* temp_left = this->left_sibling;
	NormalNode* temp_right = this->right_sibling;

	if (temp_right == NULL) {
		if (temp_left->ptr_child.size() > N_HALF_TREE)
			this->borrow_from(temp_left);
		else
			this->leaf_Merge(temp_left, root);
	}
	else if (temp_left == NULL) {
		if (temp_right->ptr_child.size() > N_HALF_TREE)
			this->borrow_from(temp_right);
		else
			this->leaf_Merge(temp_right, root);
	}
	else {
		if (temp_left->ptr_father != this->ptr_father)
			if (temp_right->ptr_child.size() > N_HALF_TREE)
				this->borrow_from(temp_right);
			else
				this->leaf_Merge(temp_right, root);
		else if (temp_right->ptr_father != this->ptr_father)
			if (temp_left->ptr_child.size() > N_HALF_TREE)
				this->borrow_from(temp_left);
			else
				this->leaf_Merge(temp_left, root);
		else {
			if (temp_left->ptr_child.size() == N_HALF_TREE&&temp_right->ptr_child.size() == N_HALF_TREE)
				this->leaf_Merge(temp_right, root);
			else if (temp_left->ptr_child.size() == N_HALF_TREE&&temp_right->ptr_child.size() > N_HALF_TREE)
				this->borrow_from(temp_right);
			else if (temp_right->ptr_child.size() == N_HALF_TREE&&temp_left->ptr_child.size() > N_HALF_TREE)
				this->borrow_from(temp_left);
			else
				this->borrow_from((temp_left->ptr_child.size() > temp_right->ptr_child.size()) ? temp_left : temp_right);
		}
	}
	return;
}

void NormalNode::borrow_from(NormalNode* temp_node) {
	NormalNode* temp_father = (NormalNode*)(this->ptr_father);
	if (temp_node == this->left_sibling) {		//borrow from left
		*(find(temp_father->key.begin(), temp_father->key.end(), this->ptr_child[0]->find_min())) = *(temp_node->key.end() - 1);
		this->key.insert(this->key.begin(), this->ptr_child[0]->find_min());
		temp_node->key.pop_back();
		this->ptr_child.insert(this->ptr_child.begin(), *(temp_node->ptr_child.end() - 1));
		temp_node->ptr_child.pop_back();
	}
	else {
		this->key.push_back(temp_node->ptr_child[0]->find_min());
		*(find(temp_father->key.begin(), temp_father->key.end(), temp_node->ptr_child[0]->find_min())) = temp_node->key[0];
		this->ptr_child.push_back(temp_node->ptr_child[0]);
		temp_node->key.erase(temp_node->key.begin());
		temp_node->ptr_child.erase(temp_node->ptr_child.begin());
	}
	return;
}

void NormalNode::leaf_Merge(NormalNode* temp_node, Node* & root) {
	unsigned int i;
	int temp_pos;
	NormalNode* temp_father = (NormalNode*)(this->ptr_father);
	NormalNode* temp_left, *temp_right;

	if (temp_node == this->left_sibling) {
		temp_left = temp_node;
		temp_right = this;
	}
	else {
		temp_left = this;
		temp_right = temp_node;
	}

	temp_pos = find(temp_father->key.begin(), temp_father->key.end(), temp_right->find_min()) - temp_father->key.begin();

	for (i = 0; i < temp_right->key.size(); i++) {
		temp_left->key.push_back(temp_right->key[i]);
		temp_left->ptr_child.push_back(temp_right->ptr_child[i]);
	}
	temp_left->ptr_child.push_back(temp_right->ptr_child[i]);

	temp_father->key.erase(temp_father->key.begin() + temp_pos);
	temp_father->ptr_child.erase(temp_father->ptr_child.begin() + temp_pos + 1);
	temp_left->right_sibling = temp_right->right_sibling;

	delete temp_right;

	if (temp_father->ptr_child.size() < N_HALF_TREE)
		if (temp_father->ptr_father != NULL)
			temp_father->node_Alter(root);
		else if (temp_father->ptr_child.size() == 1) {
			root = temp_left;
			root->ptr_father = NULL;
			delete temp_father;
		}
		else
			return;
	return;
}

string Node::find_min() {
	if (this->node_type)
		return this->key[0];
	else
		return ((NormalNode*)this)->ptr_child[0]->find_min();
}

string Node::find_max() {
	if (this->node_type)
		return *(this->key.end() - 1);
	else
		return (*(((NormalNode*)this)->ptr_child.end() - 1))->find_max();
}

void IndexManager::index_Save(string filename, Node* &root)
{

	int flag = 1;
	unsigned int i;
	unsigned int count;
	Node* temp_node = leftleaf_GET(root);
	Node* test_node = leftleaf_GET(root);
	NormalNode* temp_father = (NormalNode*)(temp_node->ptr_father);

	FILE* indexfile = fopen(filename.c_str(), "w");
	if (root == NULL || root->key.size() == 0) {
		fclose(indexfile);
		return;
	}

	fprintf(indexfile, "%d\n", root->key_type);

	while (temp_node != NULL) {
		count = 0;
		while (test_node != NULL) {
			count++;
			if (test_node->node_type)
				test_node = ((Leaf*)test_node)->right_sibling;
			else
				test_node = ((NormalNode*)test_node)->right_sibling;
		}
		fprintf(indexfile, "%d ", count);

		while (temp_node != NULL) {
			fprintf(indexfile, "%d ", temp_node->key.size());

			for (i = 0; i < temp_node->key.size(); i++) {
				fprintf(indexfile, "%s ", (*(temp_node->key.begin() + i)).c_str());
				if (flag)
					fprintf(indexfile, "%d %d ", (*(((Leaf*)temp_node)->ptr_data.begin() + i)).block_no, (*(((Leaf*)temp_node)->ptr_data.begin() + i)).tuple_no);
			}
			if (temp_node->node_type)
				temp_node = ((Leaf*)temp_node)->right_sibling;
			else
				temp_node = ((NormalNode*)temp_node)->right_sibling;
		}

		temp_node = temp_father;
		test_node = temp_father;
		if (temp_father!= NULL)
			temp_father = (NormalNode*)(temp_father->ptr_father);
		if (temp_node!=NULL)
			fprintf(indexfile, "\n");
		flag = 0;
	}

	fclose(indexfile);
}

Node* IndexManager::index_Read(string filename) {
	vector<Leaf* > temp_leaf;
	vector<NormalNode* > temp_branch;
	vector<NormalNode* > temp_branch2;
	Leaf* a_leaf;
	NormalNode* a_branch;
	vector<vector<NormalNode*> > all_branch;
	unsigned int key_num, count;
	unsigned int i;
	unsigned int j, k, flag = 1;
	char temp_string[255];
	int temp_int1;
	int temp_int2;

	int value_type;

	FILE* indexfile = fopen(filename.c_str(), "r");
	
	//Update 6.20
	if (feof(indexfile)) {
		fclose(indexfile);
		return a_leaf = new Leaf();
	}
	//Update 6.20

	fscanf(indexfile, "%d", &value_type);
	while (true) {
		fscanf(indexfile, "%d", &count);
		if (feof(indexfile))
			break;
		if (flag) {
			for (i = 0; i < count; i++) {
				fscanf(indexfile, "%d", &key_num);
				a_leaf = new Leaf();
				a_leaf->key_type = value_type;
				for (j = 0; j < key_num; j++) {
					fscanf(indexfile, "%s", &temp_string);
					fscanf(indexfile, "%d %d", &temp_int1, &temp_int2);
					a_leaf->key.push_back(temp_string);
					a_leaf->ptr_data.push_back(datum(temp_int1, temp_int2));
				}
				temp_leaf.push_back(a_leaf);
			}
			flag = 0;
		}
		else {
			for (i = 0; i < count; i++) {
				fscanf(indexfile, "%d", &key_num);
				a_branch = new NormalNode();
				a_branch->key_type = value_type;
				for (j = 0; j < key_num; j++) {
					fscanf(indexfile, "%s", &temp_string);
					a_branch->key.push_back(temp_string);
				}
				temp_branch.push_back(a_branch);
			}

			all_branch.push_back(temp_branch);
			temp_branch.clear();
		}

	}
	if (all_branch.size() == 0)
		return temp_leaf[0];
	else {
		for (i = 0; i < temp_leaf.size(); i++) {
			if (i == 0)
				temp_leaf[i]->left_sibling = NULL;
			else
				temp_leaf[i]->left_sibling = temp_leaf[i - 1];
			if (i == temp_leaf.size() - 1)
				temp_leaf[i]->right_sibling = NULL;
			else
				temp_leaf[i]->right_sibling = temp_leaf[i + 1];
		}
		for (i = 0; i < all_branch.size(); i++) {
			temp_branch = all_branch[i];
			count = 0;

			for (j = 0; j < temp_branch.size(); j++) {
				if (j == 0)
					temp_branch[j]->left_sibling = NULL;
				else
					temp_branch[j]->left_sibling = temp_branch[j - 1];
				if (j == temp_branch.size() - 1)
					temp_branch[j]->right_sibling = NULL;
				else
					temp_branch[j]->right_sibling = temp_branch[j + 1];
				if (i == 0) {
					for (k = 0; k < temp_branch[j]->key.size() + 1; k++) {
						temp_branch[j]->ptr_child.push_back(temp_leaf[count + k]);
						temp_leaf[count + k]->ptr_father = temp_branch[j];
					}
					count += temp_branch[j]->key.size()+1;
				}
				else {
					temp_branch2 = all_branch[i - 1];
					for (k = 0; k < temp_branch[j]->key.size() + 1; k++) {
						temp_branch[j]->ptr_child.push_back(temp_branch2[count + k]);
						temp_branch2[count + k]->ptr_father = temp_branch[j];
					}
					count += temp_branch[j]->key.size()+1;
				}
			}

		}
	}
	fclose(indexfile);
	return temp_branch[0];
}

void IndexManager::index_Drop(string filename, Node* root) {
	if (remove(filename.c_str()))
		cout << "Delete file " << filename << " Succeed!" << endl;
	else
		cout << "Delete file " << filename << " failed!" << endl;
	if (root == NULL) {
		cout << "Didn't have this index!" << endl;
		return;
	}
	else {
		Node* temp_leaf = leftleaf_GET(root);
		Node* temp_father = temp_leaf->ptr_father;
		Node* temp_node;
		while (temp_leaf != NULL) {
			while (temp_leaf != NULL) {
				if (temp_leaf->node_type)
					temp_node = ((Leaf*)temp_leaf)->right_sibling;
				else
					temp_node = ((NormalNode*)temp_leaf)->right_sibling;
				delete temp_leaf;
				temp_leaf = temp_node;
			}
			temp_leaf = temp_father;
			if(temp_leaf!=NULL)
				temp_father = temp_leaf->ptr_father;
		}
	}
}

tuple_Entree IndexManager::select_OnEqual(string table_name, string temp_key0, Node* root) {
	//Update 6.21
	string temp_key;
	for (unsigned int i = 0; i < temp_key0.size(); i++) {
		if (temp_key0[i] != ' ')
			temp_key.push_back(temp_key0[i]);
		else
			break;
	}

	tuple_Entree temp_result;
	int temp_pos;
	if (root->node_type) {
		if (find(root->key.begin(), root->key.end(), temp_key) != root->key.end()) {
			temp_pos = find(root->key.begin(), root->key.end(), temp_key) - root->key.begin();
			temp_result.id = ((Leaf*)root)->ptr_data[temp_pos].tuple_no;
			temp_result.block_id = ((Leaf*)root)->ptr_data[temp_pos].block_no;

			temp_result.entry_Tuple = (bm->block_GET(table_name, ((Leaf*)root)->ptr_data[temp_pos].block_no))
				->record_GET(((Leaf*)root)->ptr_data[temp_pos].tuple_no);

		}
		else {
			temp_result.entry_Tuple = "";
			temp_result.id = 0;
			temp_result.block_id = -1;
		}
	}
	else {
		temp_pos = insert_pos(root->key, temp_key, 0, root->key.size(), root->key_type);
		temp_result = select_OnEqual(table_name, temp_key, ((NormalNode*)root)->ptr_child[temp_pos]);
	}

	return temp_result;
}

vector<tuple_Entree> IndexManager::select_OnNotEqual(string table_name, string temp_key0, Node* root) {
	//Update 6.22
	string temp_key;
	for (unsigned int i = 0; i < temp_key0.size(); i++) {
		if (temp_key0[i] != ' ')
			temp_key.push_back(temp_key0[i]);
		else
			break;
	}
	vector<tuple_Entree> temp_vec;
	tuple_Entree temp_result;

	Leaf* temp_leaf= IndexManager::leftleaf_GET(root);
	while (temp_leaf != NULL) {
		for (int i = 0; i < temp_leaf->key.size(); i++) {
			if (temp_leaf->key[0] != temp_key) {
				temp_result.id = temp_leaf->ptr_data[i].tuple_no;
				temp_result.block_id = temp_leaf->ptr_data[i].block_no;

				temp_result.entry_Tuple = (bm->block_GET(table_name, temp_leaf->ptr_data[i].block_no))
					->record_GET(temp_leaf->ptr_data[i].tuple_no);
				temp_vec.push_back(temp_result);
			}
		}
		temp_leaf = temp_leaf->right_sibling;
	}
	return temp_vec;
}
//start_key must less than end_key
//if just have '<' or '>',use find_min（） or find_max（） to fit start_key or end_key
//start_flag,end_flag  means left_equal,right_equal    such as   5<=x<=8
vector<tuple_Entree> IndexManager::select_InBetween(string table_name, string start_key0, string end_key0, Node* root, int start_flag, int end_flag) {
	int temp_pos;
	vector<tuple_Entree> result_data;
	tuple_Entree temp_result;
	string start_key,end_key;
	for (unsigned int i = 0; i < start_key0.size(); i++) {
		if (start_key0[i] != ' ')
			start_key.push_back(start_key0[i]);
		else
			break;
	}
	for (unsigned int i = 0; i <end_key0.size(); i++) {
		if (end_key0[i] != ' ')
			end_key.push_back(end_key0[i]);
		else
			break;
	}

	if (root->node_type) {
		temp_pos = insert_pos(root->key, start_key, 0, root->key.size(), root->key_type);
		if (temp_pos == root->key.size()) {
			//Update 6.19
			root = ((Leaf*)root)->right_sibling;
			temp_pos = 0;
		}
		if (find(root->key.begin(), root->key.end(), start_key) != root->key.end() && start_flag != 1) {
			temp_pos += 1;
			if (temp_pos == root->key.size()) {
				root = ((Leaf*)root)->right_sibling;
				temp_pos = 0;
			}
		}
		while (root != NULL) {
			if (root->key_type == 0) {
				int a = atoi((root->key[temp_pos]).c_str());
				int b = atoi(end_key.c_str());
				if (a < b) {
					temp_result.block_id = ((Leaf*)root)->ptr_data[temp_pos].block_no;
					temp_result.id = ((Leaf*)root)->ptr_data[temp_pos].tuple_no;

					temp_result.entry_Tuple = (bm->block_GET(table_name, ((Leaf*)root)->ptr_data[temp_pos].block_no))
						->record_GET(((Leaf*)root)->ptr_data[temp_pos].tuple_no);
					result_data.push_back(temp_result);
					temp_pos++;
					if (temp_pos == root->key.size()) {
						root = ((Leaf*)root)->right_sibling;
						temp_pos = 0;
					}
				}
				else if ((root->key[temp_pos] == end_key) && end_flag) {
					temp_result.block_id = ((Leaf*)root)->ptr_data[temp_pos].block_no;
					temp_result.id = ((Leaf*)root)->ptr_data[temp_pos].tuple_no;
					temp_result.entry_Tuple = (bm->block_GET(table_name, ((Leaf*)root)->ptr_data[temp_pos].block_no))
						->record_GET(((Leaf*)root)->ptr_data[temp_pos].tuple_no);

					result_data.push_back(temp_result);
					return result_data;
				}
				else
					return result_data;
			}
			else if (root->key_type == 1) {
				double a = atof((root->key[temp_pos]).c_str());
				double b = atof(end_key.c_str());
				if (a < b) {
					temp_result.block_id = ((Leaf*)root)->ptr_data[temp_pos].block_no;
					temp_result.id = ((Leaf*)root)->ptr_data[temp_pos].tuple_no;

					temp_result.entry_Tuple = (bm->block_GET(table_name, ((Leaf*)root)->ptr_data[temp_pos].block_no))
						->record_GET(((Leaf*)root)->ptr_data[temp_pos].tuple_no);

					result_data.push_back(temp_result);
					temp_pos++;
					if (temp_pos == root->key.size()) {
						root = ((Leaf*)root)->right_sibling;
						temp_pos = 0;
					}
				}
				else if ((root->key[temp_pos] == end_key) && end_flag) {
					temp_result.block_id = ((Leaf*)root)->ptr_data[temp_pos].block_no;
					temp_result.id = ((Leaf*)root)->ptr_data[temp_pos].tuple_no;

					temp_result.entry_Tuple = (bm->block_GET(table_name, ((Leaf*)root)->ptr_data[temp_pos].block_no))
						->record_GET(((Leaf*)root)->ptr_data[temp_pos].tuple_no);

					result_data.push_back(temp_result);
					return result_data;
				}
				else
					return result_data;
			}
			else {
				string a = (root->key[temp_pos]).c_str();
				string b = end_key.c_str();
				if (a < b) {
					temp_result.block_id = ((Leaf*)root)->ptr_data[temp_pos].block_no;
					temp_result.id = ((Leaf*)root)->ptr_data[temp_pos].tuple_no;

					temp_result.entry_Tuple = (bm->block_GET(table_name, ((Leaf*)root)->ptr_data[temp_pos].block_no))
						->record_GET(((Leaf*)root)->ptr_data[temp_pos].tuple_no);
					result_data.push_back(temp_result);
					temp_pos++;
					if (temp_pos == root->key.size()) {
						root = ((Leaf*)root)->right_sibling;
						temp_pos = 0;
					}
				}
				else if ((root->key[temp_pos] == end_key) && end_flag) {
					temp_result.block_id = ((Leaf*)root)->ptr_data[temp_pos].block_no;
					temp_result.id = ((Leaf*)root)->ptr_data[temp_pos].tuple_no;

					temp_result.entry_Tuple = (bm->block_GET(table_name, ((Leaf*)root)->ptr_data[temp_pos].block_no))
						->record_GET(((Leaf*)root)->ptr_data[temp_pos].tuple_no);
					result_data.push_back(temp_result);
					return result_data;
				}
				else
					return result_data;
			}

		}
	}
	else {
		if (find(root->key.begin(), root->key.end(), start_key) != root->key.end())
			temp_pos = insert_pos(root->key, start_key, 0, root->key.size(), root->key_type) + 1;
		else
			temp_pos = insert_pos(root->key, start_key, 0, root->key.size(), root->key_type);
		vector<tuple_Entree> temp_vec_result = select_InBetween(table_name, start_key, end_key, ((NormalNode*)root)->ptr_child[temp_pos], start_flag, end_flag);
		result_data.assign(temp_vec_result.begin(), temp_vec_result.end());
	}
	return result_data;
}

Leaf* IndexManager::leftleaf_GET(Node* root) {
	if (root->node_type)
		return (Leaf*)root;
	else
		return leftleaf_GET(((NormalNode*)root)->ptr_child[0]);
}

//二分法查找key从小到大的合适插入位置，返回其数组中的下标，从0开始
int IndexManager::insert_pos(vector<string> &key, string temp_key, int start, int end, int value_type) {
	if (start == end)
		return start;
	if (value_type == 0) {
		 int a = atoi(temp_key.c_str());
		 int b = atoi((key[((start + end) / 2)]).c_str());
		if (a > b)
			return insert_pos(key, temp_key, (start + end) / 2 + 1, end, value_type);
		else if (a < b)
			return insert_pos(key, temp_key, start, (start + end) / 2, value_type);
		else
			return (start + end) / 2;
	}
	else if (value_type == 1) {
		double a = atof(temp_key.c_str());
		double b = atof((key[((start + end) / 2)]).c_str());
		if (a > b)
			return insert_pos(key, temp_key, (start + end) / 2 + 1, end, value_type);
		else if (a < b)
			return insert_pos(key, temp_key, start, (start + end) / 2, value_type);
		else
			return (start + end) / 2;
	}
	else {
		string a = temp_key.c_str();
		string b = (key[((start + end) / 2)]).c_str();
		if (a > b)
			return insert_pos(key, temp_key, (start + end) / 2 + 1, end, value_type);
		else if (a < b)
			return insert_pos(key, temp_key, start, (start + end) / 2, value_type);
		else
			return (start + end) / 2;
	}
}

bool IndexManager::index_Drop(string index_name) {
	for (unsigned int i = 0; i < cm->indices.size(); i++) {
		if (cm->indices[i].name == index_name) {
			index_Drop(index_name + ".index", cm->indices[i].root);
			cm->index_Drop(index_name);
			return 1;
		}
	}
	return 0;
}

//Mark:该函数未再次检查
string IndexManager::key_GET(char* tuple, vector<Attribute> attributes, string attr_name)
{
	int numofattr = -1;
	for (unsigned int i = 0; i < attributes.size(); i++)
	{
		if (attributes[i].name == attr_name)
		{
			numofattr = i;
			break;
		}
	}

	if (numofattr == -1) return "";
	else
	{
		int offset = 0;
		for (int i = 0; i < numofattr; i++)
			offset += attributes[i].length + 1;
		int size = attributes[numofattr].length;
		//if (numofattr != 0) offset += 1;
		char* temp = (char*)malloc(sizeof(char)*size);
		for (int i = 0; i < size; i++)
			temp[i] = tuple[offset + i];
		temp[size] = '\0';
		string s;
		char* temps = (char*)malloc(sizeof(char) * 20);
		switch (attributes[numofattr].type)
		{
		case __int:
			sprintf(temps, "%d", btoi(temp));
			s = temps;
			break;
		case __float:
			sprintf(temps, "%f", btof(temp));
			s = temps;
			break;
		case __string:
			s = temp;
			break;
		}
		//Update 6.21
		if (s.empty())
		{
			return s;
		}
		string result;
		for (unsigned int i=0; i < s.size(); i++) {
			if (s[i] !=' ')
				result.push_back(s[i]);
			else
				break;
		}
		return result;
	}
}

//update_type为0表示插入，为1表示删除
void IndexManager::index_Update(int update_type, Table* table_ptr, tuple_Entree temp_data) {
	vector<Index>::iterator temp_it;
	string temp_attr_name;
	Node* temp_root = NULL;

	for (temp_it = cm->indices.begin(); temp_it != cm->indices.end(); temp_it++) {
		if ((*temp_it).tableName == table_ptr->name) {
			temp_root = (*temp_it).root;
			temp_attr_name = (*temp_it).attrName;
		}
	}
	string temp_key = key_GET(temp_data.entry_Tuple, table_ptr->attributes, temp_attr_name);
	if (update_type == 0) {
		if (temp_root->node_type)
			((Leaf*)temp_root)->node_Insert(temp_key, datum(temp_data.block_id, temp_data.id), temp_root);
		else
			((NormalNode*)temp_root)->node_Insert(temp_key, datum(temp_data.block_id, temp_data.id), temp_root);
	}
	else {
		if (temp_root->node_type)
			((Leaf*)temp_root)->leaf_Delete(temp_key, temp_root);
		else
			((NormalNode*)temp_root)->normalnode_Delete(temp_key, temp_root);
	}
	return;
}

bool IndexManager::CreateIndex(string index_name, Table* table_ptr, string attr_name, type typ) {
	if (table_ptr == NULL)
		return 0;

	int i, j;
	string temp_key;
	Node* temp_root = new Leaf();
	temp_root->key_type = typ;
	Index* temp_index = new Index(index_name, table_ptr->name, attr_name, temp_root, typ);

	for (i = 0; i<table_ptr->blockNum; i++) {
		block* temp_block = bm->block_GET(table_ptr->name, i);
		for (j = 1; j<=temp_block->getRecordCount(); j++) {
			temp_key = key_GET(temp_block->record_GET(j), table_ptr->attributes, attr_name);
			if (temp_key == "")
				return false;
			if (temp_root->node_type)
				((Leaf*)temp_root)->node_Insert(temp_key, datum(i, j), temp_root);
			else
				((NormalNode*)temp_root)->node_Insert(temp_key, datum(i, j), temp_root);
		}
	}

	temp_index->root = temp_root;
	cm->CreateIndex(*temp_index);

	return 1;
}
