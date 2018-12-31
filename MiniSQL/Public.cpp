#include <cstring>
#include "Public.h"
using namespace std;

int btoi(const char *s)
{
	unsigned int q = 0;
	for (int i = 0; i < 8; i++)
	{
		q <<= 4;
		q += s[i] - 64;
	}
	int *t = (int*)(&q);
	return *t;
}

double btof(const char *s)
{
	unsigned long long int q = 0;
	for (int i = 0; i < 16; i++)
	{
		q <<= 4;
		q += s[i] - 64;
	}
	double *t = (double*)(&q);
	return *t;
}

char * itob(const int s)
{
	char *q = new char[9];
	unsigned int *r = (unsigned int *)(&s);
	unsigned int t = *r;
	for (int i = 7; i >= 0; i--)
	{
		q[i] = 64 + t % 16;
		t >>= 4;
	}
	q[8] = 0;
	return q;
}

char * ftob(const double s)
{
	char *q = new char[17];
	unsigned long long int *t = (unsigned long long int *)(&s);
	unsigned long long int u = *t;
	for (int i = 15; i >= 0; i--)
	{
		q[i] = 64 + u % 16;
		u >>= 4;
	}
	q[16] = 0;
	return q;
}

string index_File_GET(string name)
{
	return name + ".index";
}

string index_File_GET(string t_name, string attrib)
{
	return t_name + "_" + attrib + ".index";
}

string table_File_GET(string name)
{
	return name + ".table";
}

ifstream &operator>>(ifstream &fin, type &t)
{
	int u;
	fin >> u;
	if (u == 0) t = __int;
	else if (u == 1) t = __float;
	else if (u == 2) t = __string;
	return fin;
}

ofstream &operator<<(ofstream &fout, type &t)
{
	if (t == __int) fout << "0";
	else if (t == __float) fout << "1";
	else if (t == __string) fout << "2";
	return fout;
}

ifstream &operator>>(ifstream &fin, bool &t)
{
	char s;
	fin >> s;
	if (s == 'F') t = false;
	else if (s == 'T') t = true;
	return fin;
}

ofstream &operator<<(ofstream &fout, bool &t)
{
	if (t == true) fout << "T";
	else           fout << "F";
	return fout;
}

bool operator ==(const tuple_Entree & q, const tuple_Entree & p)
{
	return q.id == p.id && q.block_id == p.block_id && (strcmp(q.entry_Tuple, p.entry_Tuple) == 0);
}
