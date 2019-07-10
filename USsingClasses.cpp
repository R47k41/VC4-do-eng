#include ".\UsingClasses.h"

//������� ��������� �������� ��� ������:
template <>
string StrCmp<string>::set_chr_case(const string& str, int(*fnc)(int)) const
{
	using std::transform;
	if (str.empty()) return string{};
	if (fnc != tolower and fnc != toupper)
	{
		cerr << "�� ������ ���������� �������" << endl;
		return string{};
	}
	string result;
	transform(str.begin(), str.end(), inserter(result, result.begin()), fnc);
	return result;
}

//������ ��������� ����� ��� ����� ��������
template <>
bool StrCmp<string>::cmp_str(const string& a, const string& b) const
{
	string ta = set_chr_case(a);
	string tb = set_chr_case(b);
	return a < b;
}

bool TLogger::CreateLogFile(const string& fname, ofstream& log) noexcept(false)
{
	if (fname.empty())
	{
		cout << "�� ������� ��� �����!" << endl;
		return false;
	}
	log.open(fname.c_str(), ios_base::out | ios_base::trunc);
	if (!log.is_open())
		cout << "������ �������� ����� ����: " << fname << endl;
	cout << "���� ���� " << fname << " ������� ������" << endl;
	return true;
}

void TLogger::Add2Log(ofstream& log, const string& msg)
{
	if (!log)
	{
		cout << "������ ������ � ���� ����" << endl;
		return;
	}
	log << msg;
}

TLogger::TLogger(const string& rpl, const string& no_fnd, const string& m2o)
{
	CreateLogFile(rpl, rpl_log);
	CreateLogFile(no_fnd, no_fnd_log);
	CreateLogFile(m2o, many2one_log);
}

TLogger::~TLogger()
{
	if (rpl_log.is_open()) rpl_log.close();
	if (no_fnd_log.is_open()) no_fnd_log.close();
	if (many2one_log.is_open()) many2one_log.close();
}