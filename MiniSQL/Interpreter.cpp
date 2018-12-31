#include "catalogmanager.h"
#include "recordmanager.h"
#include "Interpreter.h"
#include "Public.h"
#include <vector>
#include<string>
#include<string.h>
#include<iostream>
#include <fstream>
#include <cstdlib>
using namespace std;

#define NO_CONDITION -1

Interpreter::Interpreter()
{
	cm = new CatalogManager();
	bm = new BufferManager(cm);
	im = new IndexManager(bm, cm);
	rm = new RecordManager(bm, cm, im);
}

int Interpreter::interpreter(string s)
{
	int offset = 0;//相对位置
	int l;//字符串s的长度
	unsigned int i;
	string instruction;

	l = s.length();
	while (offset < l)
	{
		instruction = getword(s, &offset);
		//第一个字符为“create”
		if (strcmp(instruction.c_str(), "create") == 0)
		{
			instruction = getword(s, &offset);
			if (strcmp(instruction.c_str(), "table") == 0)
			{
				Table table;

				table.name = getword(s, &offset);
				//判断table是否重复
				if (cm->IsTable(table.name))
				{
					cout << "Error - Table already exists" << endl;
					return 0;
				}
				instruction = getword(s, &offset);
				if (strcmp(instruction.c_str(), "(") != 0)
				{
					cout << "Syntax Error - No attributes detected for a table" << endl;
					return 0;
				}
				//存储table中的attributes信息
				else
				{
					instruction = getword(s, &offset);
					table.attrNum = 0;
					table.totalLength = 0;
					table.blockNum = 0;
					while (strcmp(instruction.c_str(), ")") != 0 && offset < l)
					{
						if (strcmp(instruction.c_str(), "primary") != 0)
						{
							Attribute attribute;

							attribute.isIndexed = attribute.isPrimaryKey = attribute.isUnique = false;
							attribute.indexName = "";
							attribute.name = instruction;
							instruction = getword(s, &offset);
							if (strcmp(instruction.c_str(), "int") == 0)
							{
								attribute.type = __int;
								attribute.length = 8;
								table.totalLength += 8;
							}
							else if (strcmp(instruction.c_str(), "char") == 0)
							{
								attribute.type = __string;
								instruction = getword(s, &offset);//getword: '('
								instruction = getword(s, &offset);
								int char_num = 0;
								for (i = 0; i < instruction.length(); i++)
								{
									char_num = char_num * 10 + instruction[i] - '0';
								}
								attribute.length = char_num;
								table.totalLength += char_num;
								instruction = getword(s, &offset);// getword: ')'
							}
							else if (strcmp(instruction.c_str(), "float") == 0)
							{
								attribute.type = __float;
								attribute.length = 16;
								table.totalLength += 16;
							}
							else
							{
								cout << "Error - Data type not supported" << endl;
								return 0;
							}
							instruction = getword(s, &offset);
							if (strcmp(instruction.c_str(), "primary") == 0)
							{
								instruction = getword(s, &offset);
								if (strcmp(instruction.c_str(), "key") == 0)
									attribute.isPrimaryKey = true;
								//调用rm模块create index
								rm->index_Create(attribute.indexName, attribute.name, table.name);
								instruction = getword(s, &offset);
							}
							if (strcmp(instruction.c_str(), "unique") == 0)
							{
								attribute.isUnique = true;
								instruction = getword(s, &offset);
							}
							table.attributes.push_back(attribute);
							table.attrNum++;
						}
						else
						{
							instruction = getword(s, &offset);
							if (strcmp(instruction.c_str(), "key") == 0)
							{
								instruction = getword(s, &offset);//getword: '('
								instruction = getword(s, &offset);
								for (i = 0; i < table.attrNum; i++)
								{
									if ((instruction.compare(table.attributes[i].name)) == 0)
									{
										table.attributes[i].isPrimaryKey = true;
										break;
									}
								}
								if (i == table.attrNum)
								{
									cout << "Error - Unable to create index for Non-Primary Key" << endl;
								}
								instruction = getword(s, &offset);//getword: ')'
							}
							instruction = getword(s, &offset);
						}
						//存储完一条attribute之后
						if (strcmp(instruction.c_str(), ",") == 0)
						{
							instruction = getword(s, &offset);
						}
						else if (strcmp(instruction.c_str(), ")") == 0);
						else
						{
							cout << "Syntax Error - No attribute detected" << endl;
							return 0;
						}
					}
					instruction = getword(s, &offset);
					//调用其它模块函数生成table
					if (strcmp(instruction.c_str(), ";") == 0)
					{
						rm->table_Create(table.name);
						cm->CreateTable(table);
						cout << "Table " << table.name << " Created" << endl;
					}
					else
					{
						cout << "Syntax Error" << endl;
						return 0;
					}
				}
			}
			//create index
			else if (strcmp(instruction.c_str(), "index") == 0)
			{
				Index index;

				index.name = getword(s, &offset);
				//判断index是否重复
				if (cm->IsIndex(index.name))
				{
					cout << "Error = Index already exists" << endl;
					return 0;
				}
				instruction = getword(s, &offset);
				if (strcmp(instruction.c_str(), "on") != 0)
				{
					cout << "Syntax Error - Table undefined" << endl;
					return 0;
				}
				else
				{
					index.tableName = getword(s, &offset);
					instruction = getword(s, &offset);
					if (strcmp(instruction.c_str(), "(") != 0)
					{
						cout << "Error - Attribute not defined" << endl;
						return 0;
					}
					else
					{
						index.attrName = getword(s, &offset);
						instruction = getword(s, &offset);
						if (strcmp(instruction.c_str(), ")") != 0)
						{
							cout << "Error - Attribute not defined" << endl;
							return 0;
						}
						else
						{
							instruction = getword(s, &offset);
							if (strcmp(instruction.c_str(), ";") != 0)
							{
								cout << "Syntax Error" << endl;
								return 0;
							}
							else
							{
								//判断attribute是否是unique
/*								Table tmp;
								int i = 0;
								tmp = cm->GetTable(index.tableName);
								while (i < tmp.attrNum-1)
								{
									if (tmp.attributes[i].name == index.name&&tmp.attributes[i].isUnique == false)
									{
										cout << "Error - Attribute not unique" << endl;
										return 0;
									}
									else
										i++;
								}*/
								//判断是否存在table和attribute
								if (cm->IsTable(index.tableName) && cm->IsAttribute(index.tableName, index.attrName))
								{
									//调用rm模块create index
									rm->index_Create(index.name, index.attrName, index.tableName);
									cout << "Index " << index.name << " on " << index.tableName << " for " << index.attrName << " created" << endl;
								}
								else
								{
									cout << "Error - Not a valid table or attribute" << endl;
									return 0;
								}
							}
						}
					}
				}
			}
		}
		//第一个字符为“drop”
		else if (strcmp(instruction.c_str(), "drop") == 0)
		{
			instruction = getword(s, &offset);
			//drop table
			if (strcmp(instruction.c_str(), "table") == 0)
			{
				string Tname;

				Tname = getword(s, &offset);
				instruction = getword(s, &offset);
				if (strcmp(instruction.c_str(), ";") != 0)
				{
					cout << "Syntax Error" << endl;
					return 0;
				}
				else
				{
					if (!cm->IsTable(Tname))
					{
						cout << "Error - Table " << Tname << "does not exist" << endl;
						return 0;
					}
					else
					{
						rm->table_Drop(Tname);
						cout << "Table " << Tname << " dropped" << endl;
					}
				}
			}
			//drop index
			else if (strcmp(instruction.c_str(), "index") == 0)
			{
				string indexname;
				string tablename;

				indexname = getword(s, &offset);
				instruction = getword(s, &offset);
				if (strcmp(instruction.c_str(), "on") != 0)
				{
					cout << "Error - Table not defined" << endl;
					return 0;
				}
				else
				{
					tablename = getword(s, &offset);
					instruction = getword(s, &offset);
					if (strcmp(instruction.c_str(), ";") != 0)
					{
						cout << "Syntax Error" << endl;
						return 0;
					}
					else
					{
						if (cm->IndexOnTable(tablename, indexname) && cm->IsTable(tablename) && cm->IsIndex(indexname))
						{
							rm->index_Drop(indexname, "/0", tablename);
							cout << "Index " << indexname << " on " << tablename << " dropped" << endl;
						}
						else
						{
							cout << "Syntax Error" << endl;
							return 0;
						}
					}
				}
			}
		}
		//第一个字符为“select”
		else if (strcmp(instruction.c_str(), "select") == 0)
		{
			vector <string> attrib_list;

			instruction = getword(s, &offset);
			while (strcmp(instruction.c_str(), "from") != 0)
			{
				if (strcmp(instruction.c_str(), ",") == 0)
					instruction = getword(s, &offset);
				//isattribute判断
				attrib_list.push_back(instruction);
				instruction = getword(s, &offset);
			}
			if (strcmp(instruction.c_str(), "from") != 0)
			{
				cout << "Error - Table not defined" << endl;
				return 0;
			}
			else
			{
				string tablename;

				tablename = getword(s, &offset);
				instruction = getword(s, &offset);
				if (strcmp(instruction.c_str(), ";") == 0)
				{
					Condition c_default(NO_CONDITION);
					int u = rm->record_Show(tablename, &attrib_list, c_default);
					cout << u << " Entry/Entries affected" << endl;
				}
				else if (strcmp(instruction.c_str(), "where") == 0)
				{
					Condition condition;
					Table table;

					table = cm->GetTable(tablename);
					//获取判断条件condition
					while (1)
					{
   					    instruction = getword(s, &offset);
						condition.attrib_Name.push_back(instruction);
						//获取attribute的类型
						type t;
						int i = 0;
						while (i < table.attrNum)
						{
							if (table.attributes[i].name == instruction)
							{
								t = table.attributes[i].type;
								break;
							}
							i++;
						}

						instruction = getword(s, &offset);
						if (strcmp(instruction.c_str(), "=") == 0)
						{
							condition.attrib_Operator.push_back(0);
							instruction = getword(s, &offset);
						}
						else if (strcmp(instruction.c_str(), "<") == 0)
						{
							instruction = getword(s, &offset);
							if (strcmp(instruction.c_str(), ">") == 0)
							{
								condition.attrib_Operator.push_back(1);
								instruction = getword(s, &offset);
							}
							else if (strcmp(instruction.c_str(), "=") == 0)
							{
								condition.attrib_Operator.push_back(4);
								instruction = getword(s, &offset);
							}
							else {
								condition.attrib_Operator.push_back(2);
							}
						}
						else if (strcmp(instruction.c_str(), ">") == 0)
						{
							instruction = getword(s, &offset);
							if (strcmp(instruction.c_str(), "=") == 0)
							{
								condition.attrib_Operator.push_back(5);
								instruction = getword(s, &offset);
							}
							else
							{
								condition.attrib_Operator.push_back(3);
							}
						}
						else
						{
							cout << "Syntax Error" << endl;
							return 0;
						}
						condition.attrib_Value.push_back(instruction);
						condition.attrib_Type.push_back(t);
						instruction = getword(s, &offset);
						if (strcmp(instruction.c_str(), ";") != 0)
						{
							if (strcmp(instruction.c_str(), "and") == 0)
								condition.attrib_Relation.push_back(__and);
							else if (strcmp(instruction.c_str(), "or") == 0)
								condition.attrib_Relation.push_back(__or);
							else
							{
								condition.attrib_Relation.push_back(__NaO);
							}
						}
						else
							break;
					}
					int u = rm->record_Show(tablename, &attrib_list, condition);
					cout << u << " Entry/Entries affected" << endl;
				}
				else
				{
					cout << "Syntax Error" << endl;
					return 0;
				}
			}
		}
		//第一个字符为“insert”
		else if (strcmp(instruction.c_str(), "insert") == 0)
		{
			instruction = getword(s, &offset);
			if (strcmp(instruction.c_str(), "into") != 0)
			{
				cout << "Error - Table not defined" << endl;
				return 0;
			}
			else
			{
				string tablename;

				tablename = getword(s, &offset);
				instruction = getword(s, &offset);
				if (strcmp(instruction.c_str(), "values") != 0)
				{
					cout << "Syntax Error" << endl;
					return 0;
				}
				else
				{
					instruction = getword(s, &offset);
					if (strcmp(instruction.c_str(), "(") != 0)
					{
						cout << "Syntax Error - Data not defined" << endl;
						return 0;
					}
					else
					{
						vector <string> value;
						vector <type> value_type;
						Table tmp;
						int i = 0;

						tmp = cm->GetTable(tablename);
						instruction = getword(s, &offset);
						while (strcmp(instruction.c_str(), ")") != 0)
						{
							value.push_back(instruction);
							value_type.push_back(tmp.attributes[i++].type);
							instruction = getword(s, &offset);
							if (strcmp(instruction.c_str(), ",") == 0)
								instruction = getword(s, &offset);
						}
						instruction = getword(s, &offset);
						if (strcmp(instruction.c_str(), ";") != 0)
						{
							cout << "Syntax Error" << endl;
							return 0;
						}
						if (rm->record_Insert(tablename, &value, &value_type) < 0)
						{
							cout << "Operation failed, check whether there is repetition on Primary Key or Unique attributes." << endl;
						}
						else
						{
							cout << "1 Entry affected" << endl;
						}
					}
				}
			}
		}
		//第一个字符为“delete”
		else if (strcmp(instruction.c_str(), "delete") == 0)
		{
			instruction = getword(s, &offset);
			if (strcmp(instruction.c_str(), "from") != 0)
			{
				cout << "Syntax error on deletion!" << endl;
				return 0;
			}
			else
			{
				string tablename;

				tablename = getword(s, &offset);
				instruction = getword(s, &offset);
				Condition condition;

				if (strcmp(instruction.c_str(), "where") != 0)
				{
					if (strcmp(instruction.c_str(), ";") != 0)
					{
						cout << "Syntax error on deletion!" << endl;
						return 0;
					}
					condition = NO_CONDITION;
				}
				else
				{
					//获取判断条件condition
					while (1)
					{
						instruction = getword(s, &offset);
						condition.attrib_Name.push_back(instruction);
						instruction = getword(s, &offset);
						if (strcmp(instruction.c_str(), "=") == 0)
						{
							condition.attrib_Operator.push_back(0);
							instruction = getword(s, &offset);
						}
						else if (strcmp(instruction.c_str(), "<") == 0)
						{
							instruction = getword(s, &offset);
							if (strcmp(instruction.c_str(), ">") == 0)
							{
								condition.attrib_Operator.push_back(1);
								instruction = getword(s, &offset);
							}
							else if (strcmp(instruction.c_str(), "=") == 0)
							{
								condition.attrib_Operator.push_back(4);
								instruction = getword(s, &offset);
							}
							else {
								condition.attrib_Operator.push_back(2);
							}
						}
						else if (strcmp(instruction.c_str(), ">") == 0)
						{
							instruction = getword(s, &offset);
							if (strcmp(instruction.c_str(), "=") == 0)
							{
								condition.attrib_Operator.push_back(5);
								instruction = getword(s, &offset);
							}
							else
							{
								condition.attrib_Operator.push_back(3);
							}
						}
						else
						{
							cout << "Syntax Error" << endl;
							return 0;
						}
						condition.attrib_Value.push_back(instruction);
						condition.attrib_Type.push_back(judge_type(instruction));
						instruction = getword(s, &offset);
						if (strcmp(instruction.c_str(), ";") != 0)
						{
							if (strcmp(instruction.c_str(), "and") == 0)
								condition.attrib_Relation.push_back(__and);
							else if (strcmp(instruction.c_str(), "or") == 0)
								condition.attrib_Relation.push_back(__or);
							else
							{
								condition.attrib_Relation.push_back(__NaO);
							}
						}
						else
							break;
					}
				}
				int u = rm->record_Delete(tablename, condition);
				cout << u << "Entry/Entries affected" << endl;
			}
		}
		//第一个字符为“quit”
		else if (strcmp(instruction.c_str(), "quit") == 0)
		{
			instruction = getword(s, &offset);
			if (strcmp(instruction.c_str(), ";") != 0)
			{
				cout << "Syntax Error!";
				return 0;
			}
			else
			{
				int i;
				for (i = 0; i < cm->indices.size(); i++)
				{
					im->index_Save((cm->indices[i].name + ".index"), cm->indices[i].root);
				}
				delete rm;
				delete im;
				delete bm;
				delete cm;
				cout << "Data saved. Now you can safely turn off your computer." << endl;
				return 255;
			}
		}
		//第一个字符为“exec”
		else if (strcmp(instruction.c_str(), "exec") == 0)
		{
			instruction = getword(s, &offset);
			if (strcmp(instruction.c_str(), "<") != 0)
			{
				cout << "Syntax Error!";
				return 0;
			}
			else
			{
				string filename;

				filename = getword(s, &offset);
				instruction = getword(s, &offset);
				if (strcmp(instruction.c_str(), ">") != 0)
				{
					cout << "File Syntax Error" << endl;
					return 0;
				}
				else
				{
					instruction = getword(s, &offset);
					if (strcmp(instruction.c_str(), ";") != 0)
					{
						cout << "Syntax Error" << endl;
						return 0;
					}
					else
					{
						ifstream fin(filename);
						string item;

						if (fin)
						{
							while (getline(fin,item))
							{
								interpreter(item);
							}
							cout << "File " << filename << " opened, reading data" << endl;
						}
						else
						{
							cout << "Error, opening file failed!" << endl;
							return 0;
						}
					}
				}
			}
		}
		else if ((strcmp(instruction.c_str(), " ")) != 0)
		{
			cout << "Syntax Error" << endl;
			return 0;
		}
	}
	cout << "File executed" << endl;

	return 1;
}

string Interpreter::getword(string s, int *offset)
{
	string instruction;
	int begin, end, l;

	l = s.length();
	while ((s[*offset] == ' ' || s[*offset] == 10 || s[*offset] == '\t' || s[*offset] == '\'') && s[*offset] != 0 && *offset < l)
	{
		(*offset)++;
	}
	if (*offset == l)
		return instruction = ' ';
	begin = end = *offset;
	while (s[end] != ' '&&s[end] != '('&&s[end] != ')'&&s[end] != ','&&s[end] != ';'
		&&s[end] != '<'&&s[end] != '>'&&s[end] != '='&&s[end] != '\'')
	{
		end++;
	}
	if (begin == end)
	{
		end++;
	}
	instruction = s.substr(begin, end - begin);
	(*offset) = end;

	return instruction;
}

type Interpreter::judge_type(string s)
{
	if (s[0] >= '0'&&s[0] <= '9')
	{
		for (int i = 0; i < s.length(); i++)
		{
			if (s[i] == '.')
				return __float;
		}
		return __int;
	}
	else
		return __string;
}