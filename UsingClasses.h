#ifndef USING_CLASSES_H
#define USING_CLASSES_H

#include <iostream>
#include <fstream>
#include <string>
#include <utility>
#include <algorithm> 
#include <iterator>


using std::ofstream;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::ios_base;
using std::inserter;
using std::ostream_iterator;

const string repl_name = ".\\repl_log.log";
const string no_fnd_name = ".\\no_fnd.log";
const string many2one_name = ".\\many2one.log";


//функтор для сравнения строк
template <class T = string>
class StrCmp
{
public:
	//функция преобразования строки в нижний регистр
	T set_chr_case(const T& str, int(*)(int) = std::tolower) const;
	//функция сравнения строк не зависимо от регистра
	bool cmp_str(const T& a, const T& b) const;
	//функтор
	bool operator()(const T& a, const T& b) const { return cmp_str(a, b); }
};

class TLogger
{
private:
	ofstream rpl_log;
	ofstream no_fnd_log;
	ofstream many2one_log;
	bool CreateLogFile(const string& fname, ofstream& log) noexcept(false);
	void Add2Log(ofstream& log, const string& msg);
public:
	TLogger(const string& rpl = repl_name, const string& no_fnd = no_fnd_name,
		const string& m2o = many2one_name);
	void Add2Rpl_log(const string& msg) { Add2Log(rpl_log, msg); };
	void Add2NoFnd_log(const string& msg) { Add2Log(no_fnd_log, msg); };
	void Add2Many_log(const string& msg) { Add2Log(many2one_log, msg); };
	~TLogger();
};
#endif // !USING_CLASSES_H
