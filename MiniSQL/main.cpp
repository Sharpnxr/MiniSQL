#include<string>
#include<string.h>
#include<iostream>
#include"Interpreter.h"
using namespace std;

int main()
{
	Interpreter IP;
	string s, word;

	while (1)
	{
		cout << ">>>";
		s = "";
		do {
			cin >> word;
			s = s + word + " ";
			word = word[word.length() - 1];
		} while (strcmp(word.c_str(), ";") != 0);
		IP.interpreter(s);
	}
	return 0;
}